LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
USE std.env.finish;

USE work.p_arithmetic.ALL;
USE work.p_decode.ALL;
USE work.p_word.ALL;
USE work.p_mem.ALL;

ENTITY cpu IS 
	GENERIC(
		sim_skip_init : boolean := false
	);
	PORT(
		clk          : IN    std_ulogic;
		nreset       : IN    std_ulogic;
		ex_interrupt : IN    std_ulogic;
		ex_int_id    : IN    std_ulogic_vector(7 DOWNTO 0);

		mem_addr     :   OUT  t_word;
		mem_data     : INOUT t_uword;
		mem_rdy      : IN    std_ulogic;
		mem_re       :   OUT std_ulogic;
		mem_we       :   OUT std_ulogic;

		port_addr    :   OUT  t_word;
		port_data    : INOUT t_uword;
		port_rdy     : IN    std_ulogic;
		port_re      :   OUT std_ulogic;
		port_we      :   OUT std_ulogic);
END ENTITY cpu;

ARCHITECTURE helium OF cpu IS 

	SIGNAL extreg_LR, extreg_SP, extreg_OF : t_reg := (x"0000", x"0000", '0');
	SIGNAL extreg_FL, extreg_F1, extreg_F2            : t_reg := (x"0000", x"0000", '0');
	--start with nop
	SIGNAL instr                                      : t_reg := (x"4800", x"4800", '0');

	--00 - internal domain, invalid
	--01 - internal domain, ucode
	--10 - external domain, regular
	--11 - external domain, interrupt
	CONSTANT exmo_ii : std_ulogic_vector(1 DOWNTO 0) := "00";
	CONSTANT exmo_iu : std_ulogic_vector(1 DOWNTO 0) := "01";
	CONSTANT exmo_er : std_ulogic_vector(1 DOWNTO 0) := "10";
	CONSTANT exmo_ei : std_ulogic_vector(1 DOWNTO 0) := "11";
	SIGNAL execution_mode, execution_mode_in  : std_ulogic_vector(1 DOWNTO 0) := exmo_iu;
	SIGNAL is_exmo_iu, is_exmo_er, is_exmo_ei : std_ulogic;
	SIGNAL exmo_switch                        : std_ulogic;

	SIGNAL interrupt_IP, regular_IP : t_reg := (x"0000", x"0000", '0'); 
	SIGNAL ucode_IP                 : t_reg := (x"0027", x"0027", '0'); 
--	SIGNAL ucode_IP                 : t_reg := (x"0000", x"0000", '0'); 
	
	SIGNAL interrupt_UI, regular_UI, ucode_UI : t_reg := (x"0000", x"0000", '0');

	SIGNAL cur_cycle, finish  , state_update : std_ulogic := '0';
	SIGNAL advance  , send_req               : std_ulogic := '1';

	SIGNAL int, int_assert, int_saved_priv : std_ulogic := '0';
	SIGNAL int_en, int_en_in               : std_ulogic := '1';
	SIGNAL int_id                          : std_ulogic_vector(7 DOWNTO 0);

	SIGNAL flag_alu : std_ulogic_vector(3 DOWNTO 0);

	SIGNAL res_alu  : t_uword;
	SIGNAL res_xrd  : t_uword;

	SIGNAL reg_in   : t_uword;

	SIGNAL rf_data, rs_data   : t_uword;

	SIGNAL operand0, operand1 : t_uword;

	SIGNAL should_jump : std_ulogic;


	SIGNAL dp_controls                            : t_controls := (OTHERS => '0');
	SIGNAL dp_rf, dp_rs, dp_alu_op, dp_jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL dp_ret_flags                           : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL dp_imm_se, dp_imm_ze                   : t_uword;
	SIGNAL dm_controls                            : t_controls := (OTHERS => '0');
	SIGNAL dm_rf, dm_rs, dm_alu_op, dm_jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL dm_ret_flags                           : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL dm_imm_se, dm_imm_ze                   : t_uword;

	SIGNAL controls                   : t_controls := (OTHERS => '0');
	SIGNAL rf, rs, alu_op, jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL ret_flags                  : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL imm_se_t, imm_ze_t         : t_uword;

	SIGNAL imm_se, imm_ze             : t_uword;


	SIGNAL FS, FO, FC, FZ             : std_ulogic;

	CONSTANT  ucode_rom    : t_mem( 63 DOWNTO 0) := (
		00 => x"A003", 01 => x"F801", 02 => x"4D07", 03 => x"5901", 04 => x"18FF",
		05 => x"9804", 06 => x"8905", 07 => x"3100", 08 => x"D801", 09 => x"4BFB",
		10 => x"2900", 11 => x"9900", 12 => x"0000", 13 => x"78F8", 14 => x"79F9",
		15 => x"A001", 16 => x"69FF", 17 => x"C001", 18 => x"2000", 19 => x"5980",
		20 => x"F800", 21 => x"4D01", 22 => x"0801", 23 => x"C101", 24 => x"7001",
		25 => x"C901", 26 => x"A000", 27 => x"7001", 28 => x"68F8", 29 => x"69F9",
		30 => x"0000", 31 => x"78F8", 32 => x"182E", 33 => x"7000", 34 => x"D802",
		35 => x"4BFD", 36 => x"68F8", 37 => x"0000", 38 => x"1801", 39 => x"9800",
		40 => x"5800",

		OTHERS => x"0000");
	SIGNAL rom_data : t_uword := x"0000";

	SIGNAL internal_mem_low  : t_mem(16#3F# DOWNTO 0) := (OTHERS => x"0000");
	SIGNAL internal_mem_high : t_mem(     7 DOWNTO 0) := (OTHERS => x"0000");
	SIGNAL imem_data : t_uword := x"0000";

	SIGNAL privileged : std_ulogic := '1';
	SIGNAL privilege_lower, privilege_raise : std_ulogic := '0';

	SIGNAL paging_en : std_ulogic := '0';
	SIGNAL pid       : t_uword := x"0000";

	SIGNAL tlb_entry, tlb_pid : t_uword;
	SIGNAL tlb_miss : std_ulogic;

	SIGNAL v_addr, p_addr  : t_uword;

	SIGNAL instr_addr, instr_addr_p1, instr_addr_next : t_uword;

	--for now like that, maybe do it better in the future if needed
	SIGNAL px_out, px_00, px_01, px_03, px_10, px_11, px_12, px_13, px_14 : t_uword := x"0000";

	SIGNAL ui_in, ui_out : t_uword;

	SIGNAL prev_ui_xwr   : std_ulogic := '0';

	SIGNAL int_saved : std_ulogic := '0';

	SIGNAL ctrl_mem_re, ctrl_mem_we, ctrl_port_re, ctrl_port_we : std_ulogic := '0';
	SIGNAL mem_access, port_access : std_ulogic := '0';

	
--	ATTRIBUTE syn_noprune : integer;
--	ATTRIBUTE syn_noprune OF dec_p: LABEL IS 1;
--	ATTRIBUTE syn_noprune OF dec_m: LABEL IS 1;
BEGIN
	regf:   ENTITY work.reg_file(arch) PORT MAP(
		clk    => clk, nreset => nreset,
		rd     => rf,      r0 => rf,      r1 => rs,
		din    => reg_in,  d0 => rf_data, d1 => rs_data,
		we     => controls.rwr AND state_update);

	alu:    ENTITY work.alu(arch)      PORT MAP(
		din0  => operand0, din1 => operand1, dout => res_alu,
		op    => alu_op,  flags => flag_alu);
	
	dec_p: ENTITY work.decoder(primary)   PORT MAP(
		signals => dp_controls,
		rf      => dp_rf,              rs => dp_rs,
		immse   => dp_imm_se,       immze => dp_imm_ze,
		jmp_flg => dp_jump_flags, ret_flg => dp_ret_flags, 
		alu_op  => dp_alu_op,
		instr   => instr.dout);
	dec_m: ENTITY work.decoder(microcode) PORT MAP(
		signals => dm_controls,
		rf      => dm_rf,              rs => dm_rs,
		immse   => dm_imm_se,       immze => dm_imm_ze,
		jmp_flg => dm_jump_flags, ret_flg => dm_ret_flags, 
		alu_op  => dm_alu_op,
		instr   => instr.dout);


	/**** memory IF ****/
	mem_addr <= p_addr;
	mem_data <= operand0 WHEN mem_we ELSE x"ZZZZ";
	--when NOT rom, access mem
	ctrl_mem_re <= NOT is_exmo_iu WHEN cur_cycle = '0'
	          ELSE controls.mrd;
	ctrl_mem_we <= '0'            WHEN cur_cycle = '0'
	          ELSE controls.mwr;

	mem_re <= ctrl_mem_re AND send_req AND NOT tlb_miss;
	mem_we <= ctrl_mem_we AND send_req AND NOT tlb_miss;

	mem_access <= ctrl_mem_re OR ctrl_mem_we;


	/**** port IF ****/
	port_addr <= operand1;
	port_data <= operand0 WHEN port_we ELSE x"ZZZZ";

	ctrl_port_re <= '0' WHEN cur_cycle = '0'
	           ELSE controls.prd;
	ctrl_port_we <= '0' WHEN cur_cycle = '0'
	           ELSE controls.pwr;

	port_re <= ctrl_port_re AND send_req;
	port_we <= ctrl_port_we AND send_req;

	port_access <= ctrl_port_re OR ctrl_port_we;

	/**** rom ****/
	rom_data <= ucode_rom(to_uint(ucode_IP.dout(5 DOWNTO 0)));

	/**** imem ****/
	--controls to make sure simulator is fine
	imem_data <= internal_mem_low (to_uint(operand1(6 DOWNTO 0))) WHEN NOT operand1(7) AND controls.ird
	        ELSE internal_mem_high(to_uint(operand1(2 DOWNTO 0))) WHEN controls.ird
	        ELSE x"0000";

	mem_imem: 
	PROCESS(ALL) IS BEGIN
		IF rising_edge(clk) THEN
			IF controls.iwr THEN
				IF operand1(7) THEN 
					internal_mem_high(to_uint(operand1(2 DOWNTO 0))) <= operand0;
				ELSE
					internal_mem_low (to_uint(operand1(6 DOWNTO 0))) <= operand0;
				END IF;
			ELSIF tlb_miss THEN
				internal_mem_high(7) <= 11x"0000" & v_addr(15 DOWNTO 11);
			END IF;
		END IF;
	END PROCESS;

	/**** instruction ****/
	instr.din <= x"4800"  WHEN NOT nreset
	        ELSE rom_data WHEN is_exmo_iu OR tlb_miss
	        ELSE mem_data;
	instr.we  <= cur_cycle ?= '0' OR tlb_miss;

	advance   <= mem_rdy  WHEN mem_access
	        ELSE port_rdy WHEN port_access
	        ELSE '1';

	finish    <= cur_cycle ?= '1' AND advance;

	state_update <= finish AND NOT tlb_miss;

	regs_instruction: 
	PROCESS (ALL) IS BEGIN
		IF rising_edge(clk) THEN
		--management is a bit more tricky so dont check for state_update
			cur_cycle <= '0'           WHEN NOT nreset 
			        ELSE '0'           WHEN advance AND tlb_miss
			        ELSE NOT cur_cycle WHEN advance 
			        ELSE cur_cycle;
			
			send_req <= '1' WHEN NOT nreset
			       ELSE advance;

			instr.dout <= instr.din WHEN instr.we OR NOT nreset ELSE instr.dout;
		END IF;
	END PROCESS;

	/**** control ****/
	imm_se <= UI_out(7 DOWNTO 0) & imm_se_t(7 DOWNTO 0) WHEN prev_ui_xwr ELSE imm_se_t;
	--cant be bigger than 8 bits so might as well extend always
	imm_ze <= UI_out(7 DOWNTO 0) & imm_ze_t(7 DOWNTO 0);

	operand0 <= rf_data;
	--jumps have special logic so can be simple here
	operand1 <= imm_ze WHEN controls.iim
	       ELSE rs_data;

	reg_in <= mem_data  WHEN controls.mrd
	     ELSE port_data WHEN controls.prd 
	     ELSE px_out    WHEN controls.pxr 
	     ELSE res_xrd   WHEN controls.xrd
	     ELSE res_alu   WHEN controls.alu
	     ELSE imem_data WHEN controls.ird
	     ELSE operand1;

	WITH rs SELECT res_xrd <=
		instr_addr_next WHEN "000",
		        x"0000" WHEN "001",
		 extreg_LR.dout WHEN "010",
		 extreg_SP.dout WHEN "011",
		 extreg_OF.dout WHEN "100",
		 extreg_FL.dout WHEN "101",
		 extreg_F1.dout WHEN "110",
		 extreg_F2.dout WHEN "111",
		  (OTHERS=>'X') WHEN OTHERS;

	WITH imm_ze(4 DOWNTO 0) SELECT px_out <=
		px_00   WHEN "00000",
		px_01   WHEN "00001",
		x"0040" WHEN "00010",
		px_03   WHEN "00011",
		px_10   WHEN "01010",
		px_11   WHEN "01011",
		px_12   WHEN "01100",
		px_13   WHEN "01101",
		px_14   WHEN "01110",
		x"0000" WHEN OTHERS;

	FS <= extreg_FL.dout(3);
	FO <= extreg_FL.dout(2);
	FC <= extreg_FL.dout(1);
	FZ <= extreg_FL.dout(0);

	PROCESS(ALL) IS 
	BEGIN
		IF is_exmo_iu THEN 
			controls   <= dm_controls;
			rf         <= dm_rf;         rs        <= dm_rs;
			alu_op     <= dm_alu_op;
			jump_flags <= dm_jump_flags; ret_flags <= dm_ret_flags;
			imm_se_t   <= dm_imm_se;     imm_ze_t  <= dm_imm_ze;
		ELSE
			controls   <= dp_controls;
			rf         <= dp_rf;         rs         <= dp_rs;
			alu_op     <= dp_alu_op;
			jump_flags <= dp_jump_flags; ret_flags  <= dp_ret_flags;
			imm_se_t   <= dp_imm_se;     imm_ze_t   <= dp_imm_ze;
		END IF;

		IF controls.jcc THEN 
			IF controls.sig THEN 
				CASE jump_flags IS 
					WHEN "000"  => should_jump <= '0';
					WHEN "001"  => should_jump <= FS ?=  FO AND NOT FZ;
					WHEN "010"  => should_jump <= FZ;
					WHEN "011"  => should_jump <= FS ?=  FO;
					WHEN "100"  => should_jump <= FS ?/= FO;
					WHEN "101"  => should_jump <= NOT FZ   ;
					WHEN "110"  => should_jump <= FS ?/=  FO OR FZ;
					WHEN "111"  => should_jump <= '1';
					WHEN OTHERS => should_jump <= '0';
				END CASE;
			ELSE 
				CASE jump_flags IS 
					WHEN "000"  => should_jump <= '0';
					WHEN "001"  => should_jump <= NOT FC AND NOT FZ;
					WHEN "010"  => should_jump <=     FZ;
					WHEN "011"  => should_jump <= NOT FC; 
					WHEN "100"  => should_jump <=     FC;
					WHEN "101"  => should_jump <= NOT FZ   ;
					WHEN "110"  => should_jump <=     FC OR      FZ; 
					WHEN "111"  => should_jump <= '1';
					WHEN OTHERS => should_jump <= '0';
				END CASE;
			END IF;
		ELSE
			should_jump <= controls.jmp;
		END IF;
	END PROCESS;

	/**** execution modes ****/

	WITH execution_mode SELECT ui_out <=
		          x"DEAD" WHEN exmo_ii,
		    ucode_UI.dout WHEN exmo_iu,
	      regular_UI.dout WHEN exmo_er,
	    interrupt_UI.dout WHEN exmo_ei,
	      (OTHERS => 'X') WHEN OTHERS;

	WITH execution_mode SELECT instr_addr <=
		          x"DEAD" WHEN exmo_ii,
		    ucode_IP.dout WHEN exmo_iu,
	      regular_IP.dout WHEN exmo_er,
	    interrupt_IP.dout WHEN exmo_ei,
	      (OTHERS => 'X') WHEN OTHERS;

	is_exmo_iu <= execution_mode ?= exmo_iu;
	is_exmo_er <= execution_mode ?= exmo_er;
	is_exmo_ei <= execution_mode ?= exmo_ei;

	exmo_switch <= execution_mode ?/= execution_mode_in;

	instr_addr_p1 <= instr_addr + 1;

	instr_addr_next <= x"0027"                WHEN NOT nreset
	              ELSE x"000D"                WHEN is_exmo_er AND tlb_miss -- entry not present 
	              ELSE x"001F"                WHEN is_exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(3)
	              ELSE px_11                  WHEN controls.irt
	              ELSE 7x"01" & int_id & '0'  WHEN int --0x0200 to 0x03FE
	              ELSE operand1               WHEN controls.xwr AND rf ?= "000"
	              ELSE extreg_LR.dout         WHEN controls.ret
	              ELSE instr_addr_p1 + imm_se WHEN controls.jmp AND controls.iim AND should_jump
	              ELSE operand0               WHEN controls.jmp AND should_jump
	              ELSE instr_addr_p1;

	--tlb miss can cause switch at each cycle
	--other than that, switch when cur_cycle=1 only
	execution_mode_in <= exmo_iu WHEN NOT nreset
				    ELSE exmo_ei WHEN finish                AND int
				    ELSE exmo_iu WHEN            is_exmo_er AND tlb_miss -- entry not present 
				    ELSE exmo_iu WHEN finish AND is_exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND (operand0(1) OR operand0(3)) --ucode routine
				    ELSE exmo_er WHEN finish AND is_exmo_ei AND int_en_in
				    ELSE exmo_er WHEN finish AND is_exmo_iu AND controls.msw 
				    ELSE execution_mode;

	regular_IP.din <= x"0000"             WHEN NOT nreset
	             ELSE regular_IP.dout     WHEN is_exmo_er AND tlb_miss
	             ELSE regular_IP.dout + 1 WHEN is_exmo_er AND controls.pxw
	             ELSE interrupt_IP.din    WHEN is_exmo_ei AND int_en_in
	             ELSE instr_addr_next;

	--has to be restored by handler from interrupt anyway
	regular_IP.we  <= (advance   AND tlb_miss)
	               OR (finish    AND is_exmo_er)
	               OR (int_en_in AND is_exmo_ei);
	
	ucode_IP.din <= instr_addr_next;
	ucode_IP.we  <= execution_mode_in ?= exmo_iu AND (finish OR tlb_miss);
	
	interrupt_IP.din <= instr_addr_next;
	interrupt_IP.we  <= finish; --dont care about overwriting 

	UI_in <= x"0000"  WHEN NOT nreset 
	    ELSE operand1 WHEN controls.xwr AND rf ?= "001"
	    ELSE x"0000";

	interrupt_UI.din <= UI_in WHEN controls.xwr AND rf ?= "001" ELSE x"0000";
	  regular_UI.din <= UI_in WHEN controls.xwr AND rf ?= "001" ELSE x"0000";
	    ucode_UI.din <= UI_in WHEN controls.xwr AND rf ?= "001" ELSE x"0000";

	interrupt_UI.we <= finish AND is_exmo_ei;
	  regular_UI.we <= finish AND is_exmo_er AND NOT tlb_miss;
	    ucode_UI.we <= finish AND is_exmo_iu;

	regs_execution_mode:	
	PROCESS(ALL) IS BEGIN
		IF rising_edge(clk) THEN
		--management is a bit more tricky so dont check for state_update
		   execution_mode <= execution_mode_in WHEN         advance OR NOT nreset ELSE    execution_mode;

		    ucode_IP.dout <=     ucode_IP.din  WHEN     ucode_IP.we OR NOT nreset ELSE     ucode_IP.dout;
		  regular_IP.dout <=   regular_IP.din  WHEN   regular_IP.we OR NOT nreset ELSE   regular_IP.dout;
		interrupt_IP.dout <= interrupt_IP.din  WHEN interrupt_IP.we OR NOT nreset ELSE interrupt_IP.dout;

		    ucode_UI.dout <=     ucode_UI.din  WHEN     ucode_UI.we OR NOT nreset ELSE     ucode_UI.dout;
		  regular_UI.dout <=   regular_UI.din  WHEN   regular_UI.we OR NOT nreset ELSE   regular_UI.dout;
		interrupt_UI.dout <= interrupt_UI.din  WHEN interrupt_UI.we OR NOT nreset ELSE interrupt_UI.dout;

		END IF;
	END PROCESS;

	/**** virtual memory ****/

	v_addr <= extreg_SP.din             WHEN cur_cycle ?= '1' AND  controls.psh
	     ELSE extreg_SP.dout            WHEN cur_cycle ?= '1' AND  controls.pop
	     ELSE operand1 + extreg_OF.dout WHEN cur_cycle ?= '1' AND (controls.mwr OR controls.mrd) AND controls.off
	     ELSE operand1                  WHEN cur_cycle ?= '1' AND (controls.mwr OR controls.mrd)
	     ELSE instr_addr;

	tlb_entry <= internal_mem_low(to_uint(v_addr(15 DOWNTO 11) & '0'));
	tlb_pid   <= internal_mem_low(to_uint(v_addr(15 DOWNTO 11) & '1'));
	tlb_miss  <= is_exmo_er AND paging_en AND (NOT tlb_entry(15) OR tlb_pid ?/= pid);
	--for now, more bits are ignored
	p_addr <= tlb_entry(4 DOWNTO 0) & v_addr(10 DOWNTO 0) WHEN NOT is_exmo_iu AND paging_en
	     ELSE v_addr;

	regs_virtual_memory:
	PROCESS(ALL) IS BEGIN 
		--can only be updated by pxw, no need to check for state update
		IF rising_edge(clk) THEN
			pid <= x"0000" WHEN NOT nreset
			  ELSE px_00 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(0)
			  ELSE pid;

			paging_en <= '0' WHEN NOT nreset
			        ELSE paging_en OR (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(0));
		END IF;
	END PROCESS;

	/*** privileged mode ****/

	--convieniently they can only happen during cycle 1 so it is fine
	privilege_lower <= (controls.irt AND int_saved_priv) 
	                OR (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(2));
	privilege_raise <=  int;

	regs_privileged:
	PROCESS(ALL) IS BEGIN
		IF rising_edge(clk) THEN
			privileged <= '0' WHEN finish AND privilege_lower
					 ELSE '1' WHEN finish AND privilege_raise
					 ELSE privileged;
		END IF;
	END PROCESS;

	
	/**** interrupts ****/

	--page fault can only be caused by int 0x01, usually in tlb miss handler 
	--there is no need to save interrupt as external interrupts are asserted until they are handled
	--and internal interrupts can happen only on second cycle
	int_id <= x"02" WHEN NOT privileged AND controls.prv
	     ELSE x"03" WHEN NOT privileged AND paging_en AND controls.mrd AND NOT tlb_entry(14) --r
	     ELSE x"03" WHEN NOT privileged AND paging_en AND controls.mwr AND NOT tlb_entry(13) --w
	     ELSE x"03" WHEN NOT privileged AND paging_en                  AND NOT tlb_entry(12) --x
	     ELSE x"06" WHEN controls.inv
	     ELSE imm_ze(7 DOWNTO 0) WHEN controls.int
	     ELSE ex_int_id;
	
	int <= (int_saved OR int_assert) AND int_en;

	int_en_in <= '1' WHEN NOT nreset OR controls.irt
	        ELSE '0' WHEN finish AND (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(5))
	        ELSE '1' WHEN finish AND (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(4))
	        ELSE '0' WHEN finish AND int
	        ELSE int_en;

	--no need to check for execute privilege
	int_assert <= controls.inv OR ex_interrupt OR controls.int
	           OR (NOT privileged AND controls.prv                                    )
	           OR (NOT privileged AND paging_en AND controls.mrd AND NOT tlb_entry(14)) --r
	           OR (NOT privileged AND paging_en AND controls.mwr AND NOT tlb_entry(13)) --w
	            ;

	regs_interrupts: 
	PROCESS(ALL) IS BEGIN
		IF rising_edge(clk) THEN
			int_en <= int_en_in;

			int_saved_priv <= '1'        WHEN NOT nreset
			             ELSE privileged WHEN int AND finish
			             ELSE int_saved_priv;

			--save some interrupts unrelaated to current instruction
			int_saved <= '0' WHEN NOT nreset OR finish
			        ELSE ex_interrupt
	                  OR (NOT privileged AND paging_en AND NOT tlb_entry(12)); --x
		END IF;
	END PROCESS;

	/**** privileged external registers ****/
	gen_sim_skip_init:
	IF sim_skip_init GENERATE
		px_03 <= x"0001";
	ELSE GENERATE
		px_03 <= x"0000";
	END GENERATE gen_sim_skip_init;

	regs_privileged_external: 
	PROCESS(ALL) IS BEGIN
		--pxw and pxr cannot cause TLB miss
		--interrupt cannot cause TLB miss
		IF rising_edge(clk) THEN 
			px_00 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "00000";

			px_01 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "00001";

			px_10 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01010"
				ELSE x"00" & int_id  WHEN int;

			px_11 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01011"
				ELSE instr_addr      WHEN int AND int_id ?= x"01" 
				ELSE instr_addr_next WHEN int;

			px_12 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01100"
				ELSE v_addr          WHEN int AND (int_id ?= x"01" OR int_id ?= x"03")
				ELSE instr.dout      WHEN int AND (int_id ?= x"02" OR int_id ?= x"06")
				ELSE x"0000"         WHEN int;

			px_13 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01101"
				ELSE x"0000"         WHEN int AND int_id ?= x"03" AND controls.mrd
				ELSE x"0001"         WHEN int AND int_id ?= x"03" AND controls.mwr
				ELSE x"0002"         WHEN int AND int_id ?= x"03" 
				ELSE x"0001"         WHEN int AND int_id ?= x"02"
				ELSE x"0000"         WHEN int;

			px_14 <= x"0000"         WHEN NOT nreset
				ELSE operand0        WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01110"
				ELSE x"0000"         WHEN int;
		END IF;
	END PROCESS;


	/**** external registers ****/

	extreg_LR.din <= x"0000"        WHEN NOT nreset
	            ELSE operand1       WHEN controls.xwr AND rf ?= "010"
	            ELSE instr_addr_p1; --cal
	extreg_LR.we  <= (controls.xwr AND rf ?= "010") OR controls.cal;

	extreg_SP.din <= x"0000"            WHEN NOT nreset
	            ELSE operand1           WHEN controls.xwr AND rf ?= "011"
	            ELSE extreg_SP.dout + 1 WHEN controls.pop
	            ELSE extreg_SP.dout + 1; --psh
	extreg_SP.we  <= (controls.xwr AND rf ?= "011") OR controls.pop OR controls.psh;

	--OF not implemented yet 
	extreg_OF.din <= x"0000"  WHEN NOT nreset
	            ELSE operand1; --condition for write
	extreg_OF.we  <= controls.xwr AND rf ?= "100";
	
	extreg_FL.din <= x"0000"              WHEN NOT nreset
	            ELSE operand1             WHEN controls.xwr AND rf ?= "101"
	            ELSE 12x"000" & ret_flags WHEN controls.ret
	            ELSE 12x"000" & flag_alu;
	extreg_FL.we  <= (controls.xwr AND rf ?= "101") OR controls.fwr;

	--F1 not implemented yet
	extreg_F1.din <= x"0000" WHEN NOT nreset
	            ELSE x"0000"; 
	extreg_F1.we  <= '0';
	--F2 not implemented yet
	extreg_F2.din <=  x"0000" WHEN NOT nreset
	             ELSE x"0000"; 
	extreg_F2.we  <= '0';

	regs_external:
	PROCESS(ALL) IS BEGIN
		IF rising_edge(clk) THEN
			extreg_LR.dout <= extreg_LR.din WHEN (extreg_LR.we AND state_update) OR NOT nreset ELSE extreg_LR.dout;
			extreg_SP.dout <= extreg_SP.din WHEN (extreg_SP.we AND state_update) OR NOT nreset ELSE extreg_SP.dout;
			extreg_OF.dout <= extreg_OF.din WHEN (extreg_OF.we AND state_update) OR NOT nreset ELSE extreg_OF.dout;
			extreg_FL.dout <= extreg_FL.din WHEN (extreg_FL.we AND state_update) OR NOT nreset ELSE extreg_FL.dout;
			extreg_F1.dout <= extreg_F1.din WHEN (extreg_F1.we AND state_update) OR NOT nreset ELSE extreg_F1.dout;
			extreg_F2.dout <= extreg_F2.din WHEN (extreg_F2.we AND state_update) OR NOT nreset ELSE extreg_F2.dout;
		END IF;
	END PROCESS;
	
	-- synthesis translate_off
	PROCESS IS BEGIN
		WAIT ON clk;

		--hcf
		IF  controls.hlt AND NOT controls.ien THEN
			WAIT FOR 2 NS;
			std.env.finish;
		END IF;
	END PROCESS;
	-- synthesis translate_on
	

END ARCHITECTURE helium;

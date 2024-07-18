LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
USE work.p_decode.ALL;
USE std.env.finish;

ENTITY cpu IS
	PORT(
		clk        : IN    std_ulogic; 
		nreset     : IN    std_ulogic; 

		mem_bus    : INOUT std_logic_vector(15 DOWNTO 0); 
		mem_addr   : OUT   std_ulogic_vector(15 DOWNTO 0); 
		mem_write  : OUT   std_ulogic; 

		port_bus   : INOUT std_logic_vector(15 DOWNTO 0); 
		port_addr  : OUT   std_ulogic_vector(15 DOWNTO 0); 
		port_write : OUT   std_ulogic; 

		ex_interrupt  : IN    std_ulogic;
		ex_int_id     : IN    std_ulogic_vector(7 DOWNTO 0));
END ENTITY cpu;

ARCHITECTURE helium OF cpu IS 
	TYPE t_reg IS RECORD 
		din  : std_ulogic_vector(15 DOWNTO 0);
		dout : std_ulogic_vector(15 DOWNTO 0);
		wen  : std_ulogic;
	END RECORD;

	TYPE t_mem IS ARRAY(natural RANGE <>) OF std_ulogic_vector(15 DOWNTO 0);

	--convienient cast
	FUNCTION add(v0 : std_ulogic_vector(15 DOWNTO 0); v1 : std_ulogic_vector(15 DOWNTO 0)) 
		RETURN std_ulogic_vector IS 
	BEGIN
		return std_ulogic_vector(unsigned(v0) + unsigned(v1));
	END FUNCTION add;
	FUNCTION sub(v0 : std_ulogic_vector(15 DOWNTO 0); v1 : std_ulogic_vector(15 DOWNTO 0)) 
		RETURN std_ulogic_vector IS 
	BEGIN
		return std_ulogic_vector(unsigned(v0) - unsigned(v1));
	END FUNCTION sub;

	SIGNAL extreg_CF, extreg_LR, extreg_SP, extreg_OF : t_reg := (x"0000", x"0000", '0');
	SIGNAL extreg_FL, extreg_F1, extreg_F2            : t_reg := (x"0000", x"0000", '0');
	--start with nop
	SIGNAL instr                                      : t_reg := (x"0000", x"B000", '0');

	--separate because 3 separate domains
	SIGNAL interrupt_IP, regular_IP, ucode_IP : t_reg := (x"FFFF", x"FFFF", '0'); 
	SIGNAL interrupt_UI, regular_UI, ucode_UI : t_reg := (x"0000", x"0000", '0');

	SIGNAL flag_alu : std_ulogic_vector(3 DOWNTO 0);

	SIGNAL res_alu  : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL res_xrd  : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL reg_in   : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL rf_data, rs_data   : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL operand0, operand1 : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL should_jump : std_ulogic;

	SIGNAL rom_out : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL dp_controls                            : t_controls := (OTHERS => '0');
	SIGNAL dp_rf, dp_rs, dp_alu_op, dp_jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL dp_ret_flags                           : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL dp_imm_se, dp_imm_ze                   : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL dm_controls                            : t_controls := (OTHERS => '0');
	SIGNAL dm_rf, dm_rs, dm_alu_op, dm_jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL dm_ret_flags                           : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL dm_imm_se, dm_imm_ze                   : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL controls                   : t_controls := (OTHERS => '0');
	SIGNAL rf, rs, alu_op, jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL ret_flags                  : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL imm_se_t, imm_ze_t         : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL imm_se, imm_ze             : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL decode_cycle_in            : std_ulogic := '0';
	SIGNAL decode_cycle               : std_ulogic := '0';


	SIGNAL FS, FO, FC, FZ             : std_ulogic;

	SIGNAL  ucode_rom    : t_mem( 63 DOWNTO 0) := (
		00=>x"0000", 01=>x"2000", 02=>x"2101", 03=>x"2202", 04=>x"7901",
		05=>x"38FF", 06=>x"3AFF", 07=>x"4100", 08=>x"D100", 09=>x"9801",
		10=>x"9A01", 11=>x"ABFB", 12=>x"3000", 13=>x"3101", 14=>x"3202",
		15=>x"0000", 16=>x"2000", 17=>x"2101", 18=>x"E801", 19=>x"C9FF",
		20=>x"8001", 21=>x"4000", 22=>x"7980", 23=>x"B800", 24=>x"AD01",
		25=>x"1801", 26=>x"8101", 27=>x"D001", 28=>x"8901", 29=>x"E800",
		30=>x"D001", 31=>x"3000", 32=>x"3101", 33=>x"0000", 34=>x"2000",
		35=>x"382F", 36=>x"D000", 37=>x"9802", 38=>x"ABFD", 39=>x"3000",
		40=>x"0000", OTHERS => x"0000");

	SIGNAL internal_mem : t_mem(511 DOWNTO 0) := (OTHERS => x"0000"); --as it should

	SIGNAL int_saved_priv_in : std_ulogic := '0';
	SIGNAL int_saved_priv : std_ulogic := '0';

	SIGNAL int_enable_in : std_ulogic := '1';
	SIGNAL int_enable    : std_ulogic := '1';

	SIGNAL privileged, privileged_in : std_ulogic := '1';

	SIGNAL paging_en, paging_en_in   : std_ulogic := '0';
	SIGNAL pid      , pid_in         : std_ulogic_vector(15 DOWNTO 0) := x"0000";

	SIGNAL privilege_lower : std_ulogic := '0';
	SIGNAL privilege_raise : std_ulogic := '0';

	SIGNAL tlb_entry, tlb_pid : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL tlb_miss : std_ulogic;

	SIGNAL v_addr, p_addr  : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL i_instr_addr, i_instr_addr_p1, i_instr_addr_next : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL e_instr_addr, e_instr_addr_p1, e_instr_addr_next : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL tr_data, tr_out : std_ulogic_vector(15 DOWNTO 0);

	--00 - internal domain, invalid
	--01 - internal domain, ucode
	--10 - external domain, regular
	--11 - external domain, interrupt
	CONSTANT exmo_ii : std_ulogic_vector(1 DOWNTO 0) := "00";
	CONSTANT exmo_iu : std_ulogic_vector(1 DOWNTO 0) := "01";
	CONSTANT exmo_er : std_ulogic_vector(1 DOWNTO 0) := "10";
	CONSTANT exmo_ei : std_ulogic_vector(1 DOWNTO 0) := "11";
	SIGNAL execution_mode, execution_mode_in : std_ulogic_vector(1 DOWNTO 0) := exmo_iu;
	SIGNAL execution_mode_we                 : std_ulogic;

	SIGNAL interrupt    : std_ulogic;
	SIGNAL interrupt_id : std_ulogic_vector(7 DOWNTO 0);

	--for now like that, maybe do it better in the future if needed
	SIGNAL px_out, px_00, px_01, px_10, px_11, px_12, px_13, px_14           : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL px_00_in, px_01_in, px_10_in, px_11_in, px_12_in, px_13_in, px_14_in : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL ui_in, ui_out, imem_out : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL prev_ui_xwr   : std_ulogic := '0';

	SIGNAL int_saved, int_saved_in : std_ulogic := '0';

BEGIN
	regf:   ENTITY work.reg_file(arch) PORT MAP(clk    => clk,
	                                            nreset => nreset,
	                                            rd     => rf,
	                                            r0     => rf,
	                                            r1     => rs,
	                                            din    => reg_in,
	                                            d0     => rf_data,
	                                            d1     => rs_data,
	                                            wen    => controls.rwr AND NOT tlb_miss);
	tregf:  ENTITY work.reg_file(arch) PORT MAP(clk    => clk,
	                                            nreset => nreset,
	                                            rd     => rf,
	                                            r0     => rf,
	                                            r1     => rs,
	                                            din    => rs_data, --only used to save stuff from RF
	                                            d0     => OPEN,
	                                            d1     => tr_data,
	                                            wen    => controls.twr);
	alu:    ENTITY work.alu(arch)      PORT MAP(din0   => operand0,
	                                            din1   => operand1,
	                                            dout   => res_alu,
	                                            op     => alu_op,
	                                            flags  => flag_alu);
	
	dec_p: ENTITY work.decoder(primary)   PORT MAP(signals => dp_controls,
	                                               rf      => dp_rf,
	                                               rs      => dp_rs,
	                                               immse   => dp_imm_se,
	                                               immze   => dp_imm_ze,
	                                               jmp_flg => dp_jump_flags,
	                                               ret_flg => dp_ret_flags, 
	                                               alu_op  => dp_alu_op,
	                                               instr   => instr.dout,
	                                               cycle   => decode_cycle);
	dec_m: ENTITY work.decoder(microcode) PORT MAP(signals => dm_controls,
	                                               rf      => dm_rf,
	                                               rs      => dm_rs,
	                                               immse   => dm_imm_se,
	                                               immze   => dm_imm_ze,
	                                               jmp_flg => dm_jump_flags,
	                                               ret_flg => dm_ret_flags, 
	                                               alu_op  => dm_alu_op,
	                                               instr   => instr.dout,
	                                               cycle   => '0'); --cycle is ignored anyway
	PROCESS(ALL) IS 
	BEGIN
		IF execution_mode ?= exmo_iu THEN 
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
	END PROCESS;
	
	--execution mode and IP management
	WITH execution_mode SELECT ui_out <=
		x"DEAD"           WHEN exmo_ii,
	    regular_UI.dout   WHEN exmo_er,
		ucode_UI.dout     WHEN exmo_iu,
	    interrupt_UI.dout WHEN exmo_ei,
	    (OTHERS => 'X')   WHEN OTHERS;

	i_instr_addr <= ucode_IP.dout;
	--IDE warns about logic loop: e_ia => tlb_entry => miss => mode_switch => e_ia
	--it is not possible since tlb miss cannot occur in interrupt mode as it is forbidden by ISA 
	--at least thats my working theory
	e_instr_addr <= interrupt_IP.dout WHEN execution_mode_in ?= exmo_ei ELSE regular_IP.dout;

	execution_mode_in <= exmo_iu WHEN NOT nreset
	                ELSE exmo_ei WHEN                               int_saved
	                ELSE exmo_iu WHEN execution_mode ?= exmo_er AND tlb_miss -- entry not present 
	                ELSE exmo_iu WHEN execution_mode ?= exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND (operand0(1) OR operand0(3)) --ucode routine
	                ELSE exmo_er WHEN execution_mode ?= exmo_ei AND int_enable_in
	                ELSE exmo_er WHEN execution_mode ?= exmo_iu AND controls.msw 
	                ELSE exmo_iu WHEN execution_mode ?= exmo_er AND controls.msw 
	                ELSE execution_mode;
	execution_mode_we <= controls.fin;

	i_instr_addr_p1   <= add(i_instr_addr, x"0001");
	i_instr_addr_next <= x"0000"                         WHEN NOT nreset
	                ELSE x"0010"                         WHEN execution_mode    ?= exmo_er AND tlb_miss -- entry not present 
	                ELSE x"0001"                         WHEN execution_mode    ?= exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(1)
	                ELSE x"0022"                         WHEN execution_mode    ?= exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(3)
	                ELSE operand1                        WHEN controls.xwr AND rf ?= "000"
	                ELSE extreg_LR.dout                  WHEN controls.ret
	                ELSE add(i_instr_addr_p1, imm_se)    WHEN controls.jmp AND controls.iim AND should_jump
	                ELSE add(i_instr_addr_p1, operand0)  WHEN controls.jmp AND should_jump
	                ELSE i_instr_addr_p1;

	e_instr_addr_p1   <= add(e_instr_addr, x"0001");
	e_instr_addr_next <= x"0000"                         WHEN NOT nreset
	                ELSE px_11                           WHEN controls.irt
	                ELSE add(interrupt_IP.dout, x"0001") WHEN execution_mode ?= exmo_ei AND int_enable_in
	                ELSE operand1                        WHEN controls.xwr AND rf ?= "000"
	                ELSE extreg_LR.dout                  WHEN controls.ret
	                ELSE add(e_instr_addr_p1, imm_se)    WHEN controls.jmp AND controls.iim AND should_jump
	                ELSE add(e_instr_addr_p1, operand0)  WHEN controls.jmp AND should_jump
	                ELSE e_instr_addr_p1;

	regular_IP.din <= x"FFFF"          WHEN NOT nreset
	             ELSE sub(regular_IP.dout, x"0001") WHEN tlb_miss
	             ELSE interrupt_IP.din              WHEN int_enable_in
	             ELSE e_instr_addr_next;
	regular_IP.wen <=  controls.fin 
	              AND (execution_mode_in ?= exmo_er OR tlb_miss);
	
	ucode_IP.din   <= x"FFFF" WHEN NOT nreset
	             ELSE x"0010" WHEN execution_mode    ?= exmo_er AND tlb_miss -- entry not present 
	             ELSE x"0001" WHEN execution_mode    ?= exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(1)
	             ELSE x"0022" WHEN execution_mode    ?= exmo_er AND controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(3)
	             ELSE i_instr_addr_next;
	ucode_IP.wen   <= controls.fin AND execution_mode_in ?= exmo_iu;

	interrupt_IP.din <= x"FFFF"                                                              WHEN NOT nreset
	               ELSE sub(internal_mem(to_integer(unsigned('1' & interrupt_id))), x"0001") WHEN int_saved_in
	               ELSE e_instr_addr_next;

	interrupt_IP.wen <= controls.fin; --dont care about overwriting 

	ui_in <= x"0000" WHEN NOT nreset 
	    ELSE operand1 WHEN controls.xwr AND rf ?= "001"
	    ELSE x"0000";

	interrupt_UI.din <= UI_in WHEN controls.xwr AND rf ?= "001" ELSE x"0000";
	regular_UI.din   <= UI_in WHEN controls.xwr AND rf ?= "001" ELSE x"0000";
	ucode_UI.din     <= UI_in WHEN controls.xwr AND rf ?= "001" ELSE x"0000";

	interrupt_UI.wen <= controls.fin AND execution_mode ?= exmo_ei;
	regular_UI.wen   <= controls.fin AND execution_mode ?= exmo_er AND NOT tlb_miss;
	ucode_UI.wen     <= controls.fin AND execution_mode ?= exmo_iu;

	--privileged mode
	privileged_in <= '0' WHEN controls.fin AND privilege_lower
	            ELSE '1' WHEN controls.fin AND privilege_raise
	            ELSE privileged;

	privilege_lower <= (controls.irt AND int_saved_priv) 
	                OR (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(2));
	privilege_raise <= (int_saved_in);

	--mem
	mem_bus   <= operand0 WHEN controls.mwr AND NOT tlb_miss
	        ELSE x"ZZZZ";
	mem_addr  <= p_addr;
	mem_write <= controls.mwr AND NOT tlb_miss;

	rom_out  <= ucode_rom(to_integer(unsigned(i_instr_addr_next(5 DOWNTO 0))));

	imem_out <= internal_mem(to_integer(unsigned(operand1(8 DOWNTO 0))));
	--virt mem 
	v_addr <= extreg_SP.din                 WHEN  controls.psh
	     ELSE extreg_SP.dout                WHEN  controls.pop
	     ELSE add(operand1, extreg_OF.dout) WHEN (controls.mwr OR controls.mrd) AND controls.off
	     ELSE operand1                      WHEN  controls.mwr OR controls.mrd
	     ELSE e_instr_addr_next;
	
	tlb_entry <= internal_mem(to_integer(unsigned(v_addr(15 DOWNTO 11) & '0')));
	tlb_pid   <= internal_mem(to_integer(unsigned(v_addr(15 DOWNTO 11) & '1')));
	tlb_miss  <= execution_mode ?= exmo_er AND paging_en AND (NOT tlb_entry(15) OR tlb_pid ?/= pid);
	--for now, more bits are ignored
	p_addr <= tlb_entry(4 DOWNTO 0) & v_addr(10 DOWNTO 0) WHEN execution_mode ?/= exmo_iu AND paging_en
	     ELSE v_addr;

	pid_in <= x"0000"  WHEN NOT nreset
	     ELSE px_00 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(0)
	     ELSE pid;

	paging_en_in <= '1' WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(0)
	           ELSE paging_en;
	--ports
	port_bus   <= operand0 WHEN controls.pwr AND NOT tlb_miss
	         ELSE x"ZZZZ";
	port_addr  <= operand1;
	port_write <= controls.pwr AND NOT tlb_miss;

	--interrupts
	int_enable_in <= '1' WHEN NOT nreset OR controls.irt
	            ELSE '0' WHEN (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(5))
	            ELSE '1' WHEN (controls.pxw AND imm_ze(4 DOWNTO 0) ?= "11111" AND operand0(4))
	            ELSE '0' WHEN int_saved_in
	            ELSE int_enable;
	
	interrupt_id <= x"01" WHEN controls.int AND imm_ze(7 DOWNTO 0) ?= x"01"
	           ELSE x"02" WHEN NOT privileged AND controls.prv
	           ELSE x"03" WHEN NOT privileged AND paging_en AND controls.mrd AND NOT tlb_entry(14) --r
	           ELSE x"03" WHEN NOT privileged AND paging_en AND controls.mwr AND NOT tlb_entry(13) --w
	           ELSE x"03" WHEN NOT privileged AND paging_en                  AND NOT tlb_entry(12) --x
	           ELSE x"05" WHEN controls.inv
	           ELSE imm_ze(7 DOWNTO 0) WHEN controls.int
	           ELSE ex_int_id;
	
	interrupt <= controls.inv OR ex_interrupt OR controls.int
	          OR (NOT privileged AND controls.prv                                    )
	          OR (NOT privileged AND paging_en AND controls.mrd AND NOT tlb_entry(14)) --r
	          OR (NOT privileged AND paging_en AND controls.mwr AND NOT tlb_entry(13)) --w
	          OR (NOT privileged AND paging_en                  AND NOT tlb_entry(12)) --x
	           ;

	int_saved_in <= interrupt AND int_enable;

	int_saved_priv_in <= '0'        WHEN NOT nreset
	                ELSE privileged WHEN int_saved_in
	                ELSE int_saved_priv;

	--privileged external registers
	px_00_in <= x"0000"  WHEN NOT nreset
	       ELSE operand0 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "00000"
	       ELSE px_00;
	px_01_in <= x"0000"  WHEN NOT nreset
	       ELSE operand0 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "00001"
	       ELSE px_01;
	px_10_in <= x"0000"  WHEN NOT nreset
	       ELSE operand0 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01010"
	       ELSE x"00" & interrupt_id WHEN int_saved_in
	       ELSE px_10;
	px_11_in <= x"0000"    WHEN NOT nreset
	       ELSE operand0   WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01011"
	       ELSE  sub(e_instr_addr, x"0001") WHEN int_saved_in AND interrupt_id ?= x"01" AND prev_ui_xwr
	       ELSE      e_instr_addr           WHEN int_saved_in AND interrupt_id ?= x"01" 
	       ELSE e_instr_addr_next           WHEN int_saved_in
	       ELSE px_11;
	px_12_in <= x"0000"    WHEN NOT nreset
	       ELSE operand0   WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01100"
	       ELSE v_addr     WHEN int_saved_in AND (interrupt_id ?= x"01" OR interrupt_id ?= x"03")
	       ELSE instr.dout WHEN int_saved_in AND (interrupt_id ?= x"02" OR interrupt_id ?= x"06")
	       ELSE x"0000"    WHEN int_saved_in
	       ELSE px_12;
	px_13_in <= x"0000"  WHEN NOT nreset
	       ELSE operand0 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01101"
	       ELSE x"0000"    WHEN int_saved_in AND interrupt_id ?= x"03" AND controls.mrd
	       ELSE x"0001"    WHEN int_saved_in AND interrupt_id ?= x"03" AND controls.mwr
	       ELSE x"0002"    WHEN int_saved_in AND interrupt_id ?= x"03" 
	       ELSE x"0001"    WHEN int_saved_in AND interrupt_id ?= x"02"
	       ELSE x"0000"    WHEN int_saved_in
	       ELSE px_13;
	px_14_in <= x"0000"  WHEN NOT nreset
	       ELSE operand0 WHEN controls.pxw AND imm_ze(4 DOWNTO 0) ?= "01110"
	       ELSE x"0000"    WHEN int_saved_in
	       ELSE px_14;

	--control
	imm_se <= UI_out(7 DOWNTO 0) & imm_se_t(7 DOWNTO 0) WHEN prev_ui_xwr ELSE imm_se_t;
	--cant be bigger than 8 bits so might as well  extend always
	imm_ze <= UI_out(7 DOWNTO 0) & imm_ze_t(7 DOWNTO 0);

	decode_cycle_in <= '0' WHEN NOT nreset 
	              ELSE '0' WHEN controls.fin
	              ELSE '1';

	operand0 <= rf_data;
	--jumps have special logic so can be simple here
	operand1 <= imm_ze WHEN controls.iim
	       ELSE rs_data;

	tr_out <= imm_ze WHEN controls.iim ELSE tr_data;

	reg_in <= mem_bus  WHEN controls.mrd
	     ELSE port_bus WHEN controls.prd 
	     ELSE px_out   WHEN controls.pxr 
	     ELSE res_xrd  WHEN controls.xrd
	     ELSE res_alu  WHEN controls.alu
	     ELSE tr_out   WHEN controls.trd
	     ELSE imem_out WHEN controls.ird
	     ELSE operand1;

	WITH rs SELECT res_xrd <=
		i_instr_addr_next WHEN "000",
		extreg_CF.dout    WHEN "001",
		extreg_LR.dout    WHEN "010",
		extreg_SP.dout    WHEN "011",
		extreg_OF.dout    WHEN "100",
		extreg_FL.dout    WHEN "101",
		extreg_F1.dout    WHEN "110",
		extreg_F2.dout    WHEN "111",
		(OTHERS=>'X')     WHEN OTHERS;

	WITH imm_ze(4 DOWNTO 0) SELECT px_out <=
		px_00          WHEN "00000",
		px_01          WHEN "00001",
		x"0040"        WHEN "00010",
		px_10          WHEN "01010",
		px_11          WHEN "01011",
		px_12          WHEN "01100",
		px_13          WHEN "01101",
		px_14          WHEN "01110",
		x"0000"        WHEN OTHERS;

	--flags
	FS <= extreg_FL.dout(3);
	FO <= extreg_FL.dout(2);
	FC <= extreg_FL.dout(1);
	FZ <= extreg_FL.dout(0);

	PROCESS(ALL) IS 
	BEGIN
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

	--regs
	instr.din <= x"B000" WHEN NOT nreset --fake almost nop for ucoderom
	        ELSE rom_out WHEN execution_mode_in ?= exmo_iu 
			ELSE mem_bus;
	instr.wen <= controls.fin OR NOT nreset;
	
	--CF is not writeable
	extreg_CF.din <= extreg_CF.dout;
	extreg_CF.wen <= '0';

	extreg_LR.din <= x"0000"        WHEN NOT nreset
	            ELSE operand1       WHEN controls.xwr AND rf ?= "010"
	            ELSE i_instr_addr_next; --cal
	extreg_LR.wen <= (controls.xwr AND rf ?= "010") OR controls.cal;

	extreg_SP.din <= x"0000"                      WHEN NOT nreset
	            ELSE operand1                     WHEN controls.xwr AND rf ?= "011"
	            ELSE add(extreg_SP.dout, x"0001") WHEN controls.pop
	            ELSE add(extreg_SP.dout, x"FFFF"); --psh
	extreg_SP.wen <= (controls.xwr AND rf ?= "011") OR controls.pop OR controls.psh;

	--OF not implemented yet 
	extreg_OF.din <= x"0000"  WHEN NOT nreset
	            ELSE operand1; --condition for write
	extreg_OF.wen <= controls.xwr AND rf ?= "100";
	
	extreg_FL.din <= x"0000"              WHEN NOT nreset
	            ELSE operand1             WHEN controls.xwr AND rf ?= "101"
	            ELSE 12x"000" & ret_flags WHEN controls.ret
	            ELSE 12x"000" & flag_alu;
	extreg_FL.wen <= (controls.xwr AND rf ?= "101") OR controls.fwr;

	--F1 not implemented yet
	extreg_F1.din <= x"0000" WHEN NOT nreset
	            ELSE x"0000"; 
	extreg_F1.wen <= '0';
	--F2 not implemented yet
	extreg_F2.din <=  x"0000" WHEN NOT nreset
	             ELSE x"0000"; 
	extreg_F2.wen <= '0';


	exregs: PROCESS (clk) IS 
	BEGIN
		IF rising_edge(clk) THEN
				extreg_CF.dout <= extreg_CF.din WHEN (extreg_CF.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_CF.dout;
				extreg_LR.dout <= extreg_LR.din WHEN (extreg_LR.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_LR.dout;
				extreg_SP.dout <= extreg_SP.din WHEN (extreg_SP.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_SP.dout;
				extreg_OF.dout <= extreg_OF.din WHEN (extreg_OF.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_OF.dout;
				extreg_FL.dout <= extreg_FL.din WHEN (extreg_FL.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_FL.dout;
				extreg_F1.dout <= extreg_F1.din WHEN (extreg_F1.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_F1.dout;
				extreg_F2.dout <= extreg_F2.din WHEN (extreg_F2.wen AND NOT tlb_miss) OR NOT nreset ELSE extreg_F2.dout;

				instr.dout     <= instr.din     WHEN instr.wen     OR NOT nreset ELSE instr.dout;

				decode_cycle   <= decode_cycle_in;

				interrupt_IP.dout <= interrupt_IP.din WHEN interrupt_IP.wen OR NOT nreset ELSE interrupt_IP.dout;
				interrupt_UI.dout <= interrupt_UI.din WHEN interrupt_UI.wen OR NOT nreset ELSE interrupt_UI.dout;
				regular_IP.dout   <= regular_IP.din   WHEN regular_IP.wen   OR NOT nreset ELSE regular_IP.dout;
				regular_UI.dout   <= regular_UI.din   WHEN regular_UI.wen   OR NOT nreset ELSE regular_UI.dout;
				ucode_IP.dout     <= ucode_IP.din     WHEN ucode_IP.wen     OR NOT nreset ELSE ucode_IP.dout;
				ucode_UI.dout     <= ucode_UI.din     WHEN ucode_UI.wen     OR NOT nreset ELSE ucode_UI.dout;


				--no need to reset on nreset, cannot be used before interrupt occurs anyway
				int_saved_priv <= int_saved_priv_in;
				int_saved <= int_saved_in;
				int_enable <= int_enable_in;


				privileged <= privileged_in;
				paging_en  <= paging_en_in;


				execution_mode <= execution_mode_in WHEN execution_mode_we OR NOT nreset ELSE execution_mode;

				pid <= pid_in;


				px_00 <= px_00_in;
				px_01 <= px_01_in;
				px_10 <= px_10_in;
				px_11 <= px_11_in;
				px_12 <= px_12_in;
				px_13 <= px_13_in;
				px_14 <= px_14_in;

				prev_ui_xwr <= nreset AND controls.xwr AND rf ?= "001";
				
				IF controls.iwr THEN
					internal_mem(to_integer(unsigned(operand1(8 DOWNTO 0)))) <= operand0;
				ELSIF tlb_miss THEN
					internal_mem(255) <= 11x"0000" & v_addr(15 DOWNTO 11);
				END IF;
		END IF;
	END PROCESS;


--simulation only
--	PROCESS IS 
--	BEGIN
--		WAIT ON clk;
--
--		--hcf
--		IF  instr.dout = x"0B00" THEN
--			WAIT FOR 2 NS;
--			finish;
--		END IF;
--	END PROCESS;

END ARCHITECTURE helium;

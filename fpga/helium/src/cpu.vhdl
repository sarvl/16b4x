LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
USE work.p_decode.ALL;

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

		interrupt  : IN    std_ulogic;
		int_id     : IN    unsigned(1 DOWNTO 0));
END ENTITY cpu;

ARCHITECTURE helium OF cpu IS 
	TYPE t_reg IS RECORD 
		din  : std_ulogic_vector(15 DOWNTO 0);
		dout : std_ulogic_vector(15 DOWNTO 0);
		wen  : std_ulogic;
	END RECORD;

	TYPE t_mem IS ARRAY(natural RANGE <>) OF std_ulogic_vector(15 DOWNTO 0);

	--mostly cast
	FUNCTION add(v0 : std_ulogic_vector(15 DOWNTO 0); v1 : std_ulogic_vector(15 DOWNTO 0)) 
		RETURN std_ulogic_vector IS 
	BEGIN
		return std_ulogic_vector(unsigned(v0) + unsigned(v1));
	END FUNCTION add;

	SIGNAL extreg_IP, extreg_CF, extreg_UI, extreg_LR : t_reg := (x"0000", x"FFFF", '0');
	SIGNAL extreg_SP, extreg_OF, extreg_FL, extreg_F1 : t_reg := (x"0000", x"0000", '0');
	SIGNAL extreg_F2                                  : t_reg := (x"0000", x"0000", '0'); 
	--start with nop
	SIGNAL instr                                      : t_reg := (x"0000", x"AF00", '0');




	SIGNAL flag_alu : std_ulogic_vector(3 DOWNTO 0);

	SIGNAL res_alu  : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL res_xrd  : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL reg_in   : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL rf_data, rs_data   : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL operand0, operand1 : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL should_jump : std_ulogic;

	SIGNAL controls                   : t_controls := (OTHERS => '0');
	SIGNAL rf, rs, alu_op, jump_flags : std_ulogic_vector(2 DOWNTO 0);
	SIGNAL ret_flags                  : std_ulogic_vector(3 DOWNTO 0);
	SIGNAL imm                        : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL imm_extended               : std_ulogic_vector(15 DOWNTO 0);
	SIGNAL decode_cycle_in            : std_ulogic := '0';
	SIGNAL decode_cycle               : std_ulogic := '0';


	SIGNAL extreg_IP_p1               : std_ulogic_vector(15 DOWNTO 0);

	SIGNAL FS, FO, FC, FZ             : std_ulogic;

	SIGNAL int_saved : std_ulogic := '0';
	SIGNAL int_id_saved : unsigned(1 DOWNTO 0) := "00";

	--8instr per interrupt
	CONSTANT int_addr : t_mem := (x"0000", x"0008", x"0010", x"0018");
	SIGNAL int_saved_IP : t_reg := (x"0000", x"0000", '1');
	SIGNAL int_saved_UI : t_reg := (x"0000", x"0000", '1');

	SIGNAL int_enable_in : std_ulogic := '1';
	SIGNAL int_enable    : std_ulogic := '1';
BEGIN
	regf:   ENTITY work.reg_file(arch) PORT MAP(clk    => clk,
	                                            nreset => nreset,
	                                            rd     => rf,
	                                            r0     => rf,
	                                            r1     => rs,
	                                            din    => reg_in,
	                                            d0     => rf_data,
	                                            d1     => rs_data,
	                                            wen    => controls.rwr);
	alu:    ENTITY work.alu(arch)      PORT MAP(din0   => operand0,
	                                            din1   => operand1,
	                                            dout   => res_alu,
	                                            op     => alu_op,
	                                            flags  => flag_alu);
	decode: ENTITY work.decoder(arch)  PORT MAP(signals => controls,
	                                            rf      => rf,
	                                            rs      => rs,
	                                            imm     => imm,
	                                            jmp_flg => jump_flags,
	                                            ret_flg => ret_flags, 
												alu_op  => alu_op,
	                                            instr   => instr.dout,
	                                            cycle   => decode_cycle);

	--mem
	mem_bus   <= operand0 WHEN controls.mwr
	        ELSE x"ZZZZ";
	mem_addr  <= extreg_SP.din  WHEN controls.psh
	        ELSE extreg_SP.dout WHEN controls.pop
	        ELSE operand1       WHEN controls.mwr OR controls.mrd
	        ELSE extreg_IP.din;
	mem_write <= controls.mwr;

	--ports
	port_bus   <= operand0 WHEN controls.pwr
	         ELSE x"ZZZZ";
	port_addr  <= operand1;
	port_write <= controls.pwr;

	--interrupts
	int_saved_IP.din <= int_saved_IP.dout               WHEN controls.irt
	               ELSE operand1                        WHEN controls.xwr AND rf ?= "000"
	               ELSE extreg_LR.dout                  WHEN controls.ret
	               ELSE add(extreg_IP_p1, imm_extended) WHEN controls.jmp AND controls.iim AND should_jump
	               ELSE add(extreg_IP_p1, operand0)     WHEN controls.jmp AND should_jump
	               ELSE extreg_IP_p1;
	int_saved_IP.wen <= int_saved;

	int_saved_UI.din <= operand1 WHEN controls.xwr AND rf ?= "001" ELSE x"0000";

	int_saved_UI.wen <= int_saved;

	int_enable_in <= '1' WHEN NOT nreset OR controls.irt
	            ELSE '0' WHEN interrupt
	            ELSE int_enable;

	--control
	imm_extended <= extreg_UI.dout(7 DOWNTO 0) & x"00" 
	             OR imm;

	decode_cycle_in <= '0' WHEN NOT nreset 
	              ELSE '0' WHEN controls.fin
	              ELSE '1';

	operand0 <= rf_data;
	operand1 <= imm_extended WHEN controls.iim
	       ELSE rs_data;


	reg_in <= mem_bus  WHEN controls.mrd
	     ELSE port_bus WHEN controls.prd 
	     ELSE res_xrd  WHEN controls.xrd
	     ELSE res_alu  WHEN controls.alu
	     ELSE operand1;

	WITH rs SELECT res_xrd <=
		extreg_IP.dout WHEN "000",
		extreg_CF.dout WHEN "001",
		extreg_LR.dout WHEN "010",
		extreg_SP.dout WHEN "011",
		extreg_OF.dout WHEN "100",
		extreg_FL.dout WHEN "101",
		extreg_F1.dout WHEN "110",
		extreg_F2.dout WHEN "111",
		(OTHERS=>'X')  WHEN OTHERS;

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
					WHEN "110"  => should_jump <= FS ?=  FO OR FZ;
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
	instr.din <= x"AF00" WHEN NOT nreset 
	        ELSE mem_bus;
	instr.wen <= controls.fin OR NOT nreset;
	
	extreg_IP_p1  <= add(extreg_IP.dout, x"0001");
	extreg_IP.din <= x"FFFF"                            WHEN NOT nreset
	            ELSE int_saved_IP.dout                  WHEN controls.irt
	            ELSE int_addr(to_integer(int_id_saved)) WHEN int_saved
	            ELSE operand1                           WHEN controls.xwr AND rf ?= "000"
	            ELSE extreg_LR.dout                     WHEN controls.ret
	            ELSE add(extreg_IP_p1, imm_extended)    WHEN controls.jmp AND controls.iim AND should_jump
	            ELSE add(extreg_IP_p1, operand0)        WHEN controls.jmp AND should_jump
	            ELSE extreg_IP_p1;
	extreg_IP.wen <= controls.fin;

	--CF is not writeable
	extreg_CF.din <= extreg_CF.dout;
	extreg_CF.wen <= '0';

	--make sure that potential instruction in interrrupt handler does not get bogus value
	extreg_UI.din <= x"0000"           WHEN NOT nreset OR int_saved
	            ELSE int_saved_UI.dout WHEN controls.irt
	            ELSE operand1          WHEN controls.xwr AND rf ?= "001"
	            ELSE x"0000";
	extreg_UI.wen <= '1';

	extreg_LR.din <= x"0000"        WHEN NOT nreset
	            ELSE operand1       WHEN controls.xwr AND rf ?= "010"
	            ELSE extreg_IP_p1; --cal
	extreg_LR.wen <= (controls.xwr AND rf ?= "010") OR controls.cal;

	extreg_SP.din <= x"0000"                      WHEN NOT nreset
	            ELSE operand1                     WHEN controls.xwr AND rf ?= "011"
	            ELSE add(extreg_SP.dout, x"0001") WHEN controls.pop
	            ELSE add(extreg_SP.dout, x"FFFF"); --psh
	extreg_SP.wen <= (controls.xwr AND rf ?= "011") OR controls.pop OR controls.psh;

	--OF not implemented yet 
	extreg_OF.din <= x"0000" WHEN NOT nreset
	            ELSE x"0000"; 
	extreg_OF.wen <= '0';
	
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
				extreg_IP.dout <= extreg_IP.din WHEN extreg_IP.wen OR NOT nreset ELSE extreg_IP.dout;
				extreg_CF.dout <= extreg_CF.din WHEN extreg_CF.wen OR NOT nreset ELSE extreg_CF.dout;
				extreg_UI.dout <= extreg_UI.din WHEN extreg_UI.wen OR NOT nreset ELSE extreg_UI.dout;
				extreg_LR.dout <= extreg_LR.din WHEN extreg_LR.wen OR NOT nreset ELSE extreg_LR.dout;
				extreg_SP.dout <= extreg_SP.din WHEN extreg_SP.wen OR NOT nreset ELSE extreg_SP.dout;
				extreg_OF.dout <= extreg_OF.din WHEN extreg_OF.wen OR NOT nreset ELSE extreg_OF.dout;
				extreg_FL.dout <= extreg_FL.din WHEN extreg_FL.wen OR NOT nreset ELSE extreg_FL.dout;
				extreg_F1.dout <= extreg_F1.din WHEN extreg_F1.wen OR NOT nreset ELSE extreg_F1.dout;
				extreg_F2.dout <= extreg_F2.din WHEN extreg_F2.wen OR NOT nreset ELSE extreg_F2.dout;

				instr.dout     <= instr.din     WHEN instr.wen     OR NOT nreset ELSE instr.dout;

				decode_cycle   <= decode_cycle_in;


				--no need to reset on nreset, cannot be used before interrupt occurs anyway
				int_saved_IP.dout <= int_saved_IP.din WHEN int_saved_IP.wen ELSE int_saved_IP.dout;
				int_saved_UI.dout <= int_saved_UI.din WHEN int_saved_UI.wen ELSE int_saved_UI.dout;


				int_saved    <= '1' WHEN interrupt AND int_enable
				           ELSE '0' WHEN controls.fin
				           ELSE int_saved; 
				int_id_saved <= int_id WHEN interrupt
				           ELSE int_id_saved; 
				int_enable <= int_enable_in;
		END IF;
	END PROCESS;

END ARCHITECTURE helium;

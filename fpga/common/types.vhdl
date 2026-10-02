LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

PACKAGE pkg_types IS 
	SUBTYPE t_ubyte     IS std_ulogic_vector( 7 DOWNTO 0);
	SUBTYPE t_rbyte     IS std_logic_vector ( 7 DOWNTO 0);
	SUBTYPE t_byte      IS t_ubyte;

	SUBTYPE t_uword     IS std_ulogic_vector(15 DOWNTO 0);
	SUBTYPE t_rword     IS std_logic_vector (15 DOWNTO 0);
	SUBTYPE t_word      IS t_uword;

	SUBTYPE t_udword    IS std_ulogic_vector(31 DOWNTO 0);
	SUBTYPE t_rdword    IS std_logic_vector (31 DOWNTO 0);
	SUBTYPE t_dword     IS t_udword;

	SUBTYPE t_rmemaddr IS std_logic_vector(15 DOWNTO 0);
	SUBTYPE t_umemaddr IS std_ulogic_vector(15 DOWNTO 0);
	SUBTYPE t_memaddr  IS t_umemaddr;

	TYPE t_b_arr  IS ARRAY(natural RANGE<>) OF t_byte;
	TYPE t_w_arr  IS ARRAY(natural RANGE<>) OF t_word;
	TYPE t_dw_arr IS ARRAY(natural RANGE<>) OF t_dword;
END PACKAGE pkg_types;

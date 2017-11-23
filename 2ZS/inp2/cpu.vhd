-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2015 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Andrej Barna (xbarna01@stud.fit.vutbr.cz)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

 type fsm_state is (sidle, sfetch, sdecode, sptrinc, sptrdec, sraminc, sraminc_set, sraminc_load,
                    sramdec, sramdec_set, sramdec_load, sreturn, sothers, swhileb, swhileb_cond,
                    swhileb_loop, swhileb_check, swhilee, swhilee_cond, swhilee_wait, swhilee_loop,
                    swhilee_cntcond, swhilee_check, sgetchr, sputchr, sputchr_transfer, sgettmp,
                    sputtmp, sputtmp_set);

 signal pstate: fsm_state;
 signal nstate: fsm_state;

 signal cnt_reg: std_logic_vector(7 downto 0);  -- vnorenie cyklu while, 256 snad staci
 signal cnt_inc: std_logic;
 signal cnt_dec: std_logic;
 signal cnt_zero: std_logic;
 signal cnt_force_one: std_logic;               -- Pomocny signal pre vlozenie 1 do CNT

 signal tmp_reg: std_logic_vector(7 downto 0);  -- v TMP sa docasne skladuje hodnota bunky (8b)
 signal tmp_ld: std_logic;

 signal pc_reg: std_logic_vector(12 downto 0);  -- PC a PTR su oba registre obsahujuce adresu do pamati, tj. 13b
 signal pc_inc: std_logic;
 signal pc_dec: std_logic;

 signal ptr_reg: std_logic_vector(12 downto 0);
 signal ptr_inc: std_logic;
 signal ptr_dec: std_logic;

 signal mx1_sel: std_logic;
 signal mx2_sel: std_logic_vector(1 downto 0);

 signal data_rdata_zero: std_logic := '1';


begin


	-- CNT
	process(RESET, CLK)
	begin
	   if RESET = '1' then
	      cnt_reg <= "00000000";
	   elsif CLK'event and CLK = '1' then
	      if cnt_force_one = '1' then
	         cnt_reg <= "00000001";
	      else
	         if cnt_inc = '1' then
	            cnt_reg <= cnt_reg + '1';
	         elsif cnt_dec = '1' then
	            cnt_reg <= cnt_reg - '1';
	         end if;
	      end if;
	   end if;
	end process;
  
  
   -- CNT zero watcher
   process(cnt_reg)
   begin
	  if cnt_reg = "00000000" then
	     cnt_zero <= '1';
      else
	     cnt_zero <= '0';
      end if;
   end process;


	-- TMP
	process(RESET, CLK)
	begin
	   if RESET = '1' then
	      tmp_reg <= "00000000";
	   elsif CLK'event and CLK = '1' then
	      if tmp_ld = '1' then
	         tmp_reg <= DATA_RDATA;
	      end if;
	   end if;
	end process;


	-- PC
	process(RESET, CLK)
	begin
	   if RESET = '1' then
	      pc_reg <= "0000000000000";
	   elsif CLK'event and CLK = '1' then
	      if pc_inc = '1' then
	         pc_reg <= pc_reg + '1';
	      elsif pc_dec = '1' then
	         pc_reg <= pc_reg - '1';
	      end if;
	   end if;
	end process;


	-- PTR
	process(RESET, CLK)
	begin
	   if RESET = '1' then
	      ptr_reg <= "1000000000000";
	   elsif CLK'event and CLK = '1' then
	      if ptr_inc = '1' then
	         ptr_reg <= ptr_reg + '1';
	      elsif ptr_dec = '1' then
	         ptr_reg <= ptr_reg - '1';
	      end if;
	   end if;
	end process;


	-- MX1 = adresovy
	process(pc_reg, ptr_reg, mx1_sel)
	begin
	   case mx1_sel is
	      when '1'    => DATA_ADDR <= ptr_reg;
	      when others => DATA_ADDR <= pc_reg;
	   end case;
	end process;


	-- MX2 = zapisovy
	process(IN_DATA, tmp_reg, DATA_RDATA, mx2_sel)
	begin
	   case mx2_sel is
	      when "00"   => DATA_WDATA <= IN_DATA;
	      when "01"   => DATA_WDATA <= tmp_reg;
	      when "10"   => DATA_WDATA <= DATA_RDATA - '1';
	      when others => DATA_WDATA <= DATA_RDATA + '1';
	   end case;
	end process;


	-- DATA_RDATA handler
	process(DATA_RDATA)
	begin
     OUT_DATA <= DATA_RDATA;
	   if DATA_RDATA = "00000000" then
	      data_rdata_zero <= '1';
	   else
	      data_rdata_zero <= '0';
	   end if;
	end process;

--------------------------------------------------------------------------------------------------------------------------------

   -- =================================================================
   -- FSM present state
   -- =================================================================
   fsm_pstate: process(RESET, CLK)
   begin
      if RESET = '1' then
         pstate <= sidle;
      elsif CLK'event and CLK = '1' then
         if EN = '1' then      
            pstate <= nstate;
         end if;
      end if;
   end process;

   -- =================================================================
   -- FSM next state logic, Output logic (Moore FSM)
   -- =================================================================
   process(pstate, DATA_RDATA, data_rdata_zero, cnt_zero, IN_VLD, OUT_BUSY)
   begin
      nstate <= sidle;

      -- Registre
      cnt_inc <= '0';
      cnt_dec <= '0';
      cnt_force_one <= '0';
      tmp_ld <= '0';
      pc_inc <= '0';
      pc_dec <= '0';
      ptr_inc <= '0';
      ptr_dec <= '0';

      -- Multiplexory
      mx1_sel <= '1';  -- Default PTR
      mx2_sel <= "11"; -- Default DATA_RDATA+1

      -- I/O
      IN_REQ <= '0';
      OUT_WE <= '0';

      -- RAM
      DATA_RDWR <= '1'; -- Default Read Mode
      DATA_EN <= '0';


      case pstate is
         -- IDLE
         when sidle =>
            nstate <= sfetch;

         -- INSTRUCTION FETCH
         when sfetch =>
            nstate <= sdecode; 
            mx1_sel <= '0';
            DATA_EN <= '1';
         
         -- INSTRUCTION DECODE
         when sdecode =>
            case DATA_RDATA is
               when X"3E"  => nstate <= sptrinc; -- >   State Pointer Increment
               when X"3C"  => nstate <= sptrdec; -- <   State Pointer Decrement
               when X"2B"  => nstate <= sraminc; -- +   State RAM Increment
               when X"2D"  => nstate <= sramdec; -- -   State RAM Decrement
               when X"2E"  => nstate <= sputchr; -- .   State Put Character
               when X"2C"  => nstate <= sgetchr; -- ,   State Get Character
               when X"5B"  => nstate <= swhileb; -- [   State While - Begin
               when X"5D"  => nstate <= swhilee; -- ]   State While - End
               when X"24"  => nstate <= sputtmp; -- $   State Put Into Temporary
               when X"21"  => nstate <= sgettmp; -- !   State Get From Temporary
               when X"00"  => nstate <= sreturn; -- \0  State Return
               when others => nstate <= sothers; --     State Other Characters
            end case;

-------------------------------------------------------------------------------
         
         -- NULL = \0
         when sreturn =>
            nstate <= sreturn;

-------------------------------------------------------------------------------

         -- Others = Komentare
         when sothers =>
            nstate <= sfetch;
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- Pointer Increment = >
         when sptrinc =>
            nstate <= sfetch;
            ptr_inc <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- Pointer Decrement = <
         when sptrdec =>
            nstate <= sfetch;
            ptr_dec <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- RAM Increment - Nacitanie aktualnej bunky
         when sraminc =>
            nstate <= sraminc_set;
            DATA_EN <= '1';

         -- RAM Increment - Ulozenie inkrementovanej hodnoty
         when sraminc_set =>
            nstate <= sraminc_load;
            DATA_RDWR <= '0';
            DATA_EN <= '1';

         -- RAM Increment - Aktualizovanie hodnoty na DATA_RDATA
         when sraminc_load =>
            nstate <= sfetch;
            DATA_EN <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- RAM Decrement - Nacitanie aktualnej bunky
         when sramdec =>
            nstate <= sramdec_set;
            DATA_EN <= '1';

         -- RAM Decrement - Ulozenie dekrementovanej hodnoty
         when sramdec_set =>
            nstate <= sramdec_load;
            mx2_sel <= "10";
            DATA_RDWR <= '0';
            DATA_EN <= '1';

         -- RAM Decrement - Aktualizovanie hodnoty na DATA_RDATA
         when sramdec_load =>
            nstate <= sfetch;
            DATA_EN <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- Put Into Temporary
         when sputtmp =>
            nstate <= sputtmp_set;
            DATA_EN <= '1';

         -- Put Into Temporary
         when sputtmp_set =>
            nstate <= sfetch;
            tmp_ld <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- Get From Temporary
         when sgettmp =>
            nstate <= sfetch;
            mx2_sel <= "01";
            DATA_RDWR <= '0';
            DATA_EN <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- Put Character - Cakanie a nasledne nacitanie dat
         when sputchr =>
            if OUT_BUSY = '1' then
               nstate <= sputchr;
            else
               nstate <= sputchr_transfer;
               DATA_EN <= '1';
            end if;
            
         -- Put Character - Vypis dat
         when sputchr_transfer =>
            nstate <= sfetch;
            OUT_WE <= '1';
            pc_inc <= '1';

-------------------------------------------------------------------------------

         -- Get Character - Cakanie na vstup a nasledny zapis dat
         when sgetchr =>
            IN_REQ <= '1';
            if IN_VLD = '0' then
               nstate <= sgetchr;
            else
               mx2_sel <= "00";
               DATA_RDWR <= '0';
               DATA_EN <= '1';
               pc_inc <= '1';
               nstate <= sfetch;
            end if;

-------------------------------------------------------------------------------

         -- While Begin - Inkrementovanie PC a nacitanie RAM[PTR]
         when swhileb =>
            pc_inc <= '1';
            DATA_EN <= '1';
            nstate <= swhileb_cond;
         
         -- While Begin - Kontrola nulovosti aktualnej bunky
         when swhileb_cond =>
            mx1_sel <= '0';
            DATA_EN <= '1';
            if data_rdata_zero = '1' then
               cnt_force_one <= '1';
               nstate <= swhileb_loop;
            else
               nstate <= sdecode;
            end if;

         -- While Begin - Preskocenie bunky, modifikovanie hodnoty CNT
         when swhileb_loop =>
            nstate <= swhileb_check;
            pc_inc <= '1';
            if DATA_RDATA = X"5B" then
               cnt_inc <= '1';
            elsif DATA_RDATA = X"5D" then
               cnt_dec <= '1';
            end if;

         -- While Begin - Kontrola podmienky cyklu while
         when swhileb_check =>
            mx1_sel <= '0';
            DATA_EN <= '1';
            if cnt_zero = '1' then
               nstate <= sdecode;
            else
               nstate <= swhileb_loop;
            end if;

-------------------------------------------------------------------------------

         -- While End - Nacitanie hodnoty RAM[PTR]
         when swhilee =>
            DATA_EN <= '1';
            nstate <= swhilee_cond;

         -- While End - Kontrola nulovosti aktualnej bunky
         when swhilee_cond =>
            if data_rdata_zero = '1' then
               pc_inc <= '1';
               nstate <= sfetch;
            else
               cnt_force_one <= '1';
               pc_dec <= '1';
               nstate <= swhilee_wait;
            end if;

         -- While End - Pockali sme na aktualizovanie PC
         when swhilee_wait =>
            nstate <= swhilee_loop;
            mx1_sel <= '0';
            DATA_EN <= '1';

         -- While End - Preskocenie bunky, modifikovanie hodnoty CNT
         when swhilee_loop =>
            nstate <= swhilee_cntcond;
            if DATA_RDATA = X"5B" then
               cnt_dec <= '1';
            elsif DATA_RDATA = X"5D" then
               cnt_inc <= '1';
            end if;

         -- While End - Kontrola podmienky CNT a nasledna inkr./dekr. PC
         when swhilee_cntcond =>
            nstate <= swhilee_check;
            if cnt_zero = '1' then
               pc_inc <= '1';
            else
               pc_dec <= '1';
            end if;

         -- While End - Kontrola podmienky cyklu while
         when swhilee_check =>
            mx1_sel <= '0';
            DATA_EN <= '1';
            if cnt_zero = '1' then
               nstate <= sdecode;
            else
               nstate <= swhilee_loop;
            end if;

-------------------------------------------------------------------------------

         when others =>
      end case;
   end process;

end behavioral;
 

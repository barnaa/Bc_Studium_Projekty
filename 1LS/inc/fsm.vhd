-- fsm.vhd: Pristupovy terminal
-- Autor: Andrej Barna (xbarna01)
-- Kod1 = 2009722555 	 Kod2 = 2003611577

library ieee;
use ieee.std_logic_1164.all;
-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity fsm is
port(
   CLK         : in  std_logic;
   RESET       : in  std_logic;

   -- Input signals
   KEY         : in  std_logic_vector(15 downto 0);
   CNT_OF      : in  std_logic;

   -- Output signals
   FSM_CNT_CE  : out std_logic;
   FSM_MX_MEM  : out std_logic;
   FSM_MX_LCD  : out std_logic;
   FSM_LCD_WR  : out std_logic;
   FSM_LCD_CLR : out std_logic
);
end entity fsm;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of fsm is
   type t_state is (TEST0, TEST1, TEST2, TEST3, TEST14, TEST15, TEST16, TEST17, TEST18, TEST19, TEST24, TEST25, TEST26, TEST27, TEST28, TEST29, PRINT_MESSAGE_AD, PRINT_MESSAGE_AG, STATE_WR, STATE_GD, FINISH);
   signal present_state, next_state : t_state;

begin
-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= TEST0;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- -------------------------------------------------------
next_state_logic : process(present_state, KEY, CNT_OF)
begin
   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST0 =>
      next_state <= TEST0;
      if (KEY(2) = '1') then
         next_state <= TEST1;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST1 =>
      next_state <= TEST1;
      if (KEY(0) = '1') then
         next_state <= TEST2;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST2 =>
      next_state <= TEST2;
      if (KEY(0) = '1') then
         next_state <= TEST3;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST3 =>
      next_state <= TEST3;
      if (KEY(9) = '1') then
         next_state <= TEST14;
      elsif (KEY(3) = '1') then
         next_state <= TEST24;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
-- ------------------------------------------------
-- Zvysok prveho kodu
-- ------------------------------------------------
   when TEST14 =>
      next_state <= TEST14;
      if (KEY(7) = '1') then
         next_state <= TEST15;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST15 =>
      next_state <= TEST15;
      if (KEY(2) = '1') then
         next_state <= TEST16;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST16 =>
      next_state <= TEST16;
      if (KEY(2) = '1') then
         next_state <= TEST17;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST17 =>
      next_state <= TEST17;
      if (KEY(5) = '1') then
         next_state <= TEST18;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST18 =>
      next_state <= TEST18;
      if (KEY(5) = '1') then
         next_state <= TEST19;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST19 =>
      next_state <= TEST19;
      if (KEY(5) = '1') then
         next_state <= STATE_GD;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -

-- ------------------------------------------------
-- Zvysok druheho kodu
-- ------------------------------------------------
   when TEST24 =>
      next_state <= TEST24;
      if (KEY(6) = '1') then
         next_state <= TEST25;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST25 =>
      next_state <= TEST25;
      if (KEY(1) = '1') then
         next_state <= TEST26;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST26 =>
      next_state <= TEST26;
      if (KEY(1) = '1') then
         next_state <= TEST27;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST27 =>
      next_state <= TEST27;
      if (KEY(5) = '1') then
         next_state <= TEST28;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST28 =>
      next_state <= TEST28;
      if (KEY(7) = '1') then
         next_state <= TEST29;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TEST29 =>
      next_state <= TEST29;
      if (KEY(7) = '1') then
         next_state <= STATE_GD;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   -- -------------------------------------------
   -- - - - - - - - - - - - - - - - - - - - - - -
   when STATE_WR =>
      next_state <= STATE_WR;
      if (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AD; 
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when STATE_GD =>
      next_state <= STATE_GD;
      if (KEY(15) = '1') then
         next_state <= PRINT_MESSAGE_AG;
      elsif (KEY(15 downto 0) /= "0000000000000000") then
         next_state <= STATE_WR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_MESSAGE_AD =>
      next_state <= PRINT_MESSAGE_AD;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_MESSAGE_AG =>
      next_state <= PRINT_MESSAGE_AG;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINISH =>
      next_state <= FINISH;
      if (KEY(15) = '1') then
         next_state <= TEST0; 
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      next_state <= TEST0;
   end case;
end process next_state_logic;



-- -------------------------------------------------------
output_logic : process(present_state, KEY)
begin
   FSM_CNT_CE     <= '0';
   FSM_MX_MEM     <= '0';
   FSM_MX_LCD     <= '0';
   FSM_LCD_WR     <= '0';
   FSM_LCD_CLR    <= '0';

   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_MESSAGE_AG =>
      FSM_CNT_CE     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_MX_MEM     <= '1';
      FSM_LCD_WR     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_MESSAGE_AD =>
      FSM_CNT_CE     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINISH =>
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      if (KEY(14 downto 0) /= "000000000000000") then
         FSM_LCD_WR     <= '1';
      end if;
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   end case;
end process output_logic;

end architecture behavioral;


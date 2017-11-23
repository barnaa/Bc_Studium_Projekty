library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;



entity ledc8x8 is
  port (
    RESET, SMCLK: in STD_LOGIC;
    ROW, LED: out STD_LOGIC_VECTOR (7 downto 0)
  );
end entity ledc8x8;



architecture behv of ledc8x8 is

  -- Pocitadlo casu
   signal counter: STD_LOGIC_VECTOR (22 downto 0) := "00000000000000000000000";
  
  -- Registre pre vystupy ROW a LED
   signal led_content: STD_LOGIC_VECTOR (7 downto 0) := "11111110";
   signal row_content: STD_LOGIC_VECTOR (7 downto 0) := "00000000";
  
  -- Aktivuje sa pri kazdom 256. takte, cim sa zapne dalsi riadok na displeji
   signal refresh: STD_LOGIC;
  
  -- Nastavi sa na jednotku, ked je splnena casova podmienka T>0.5s
  -- Konkretne 2^22*125ns je cca 0.524s 
   signal condmet: STD_LOGIC;
  
  
  
  begin   
           
  -- Citac          
  process(RESET,SMCLK) is
  begin
    if (RESET='1') then
      counter <= "00000000000000000000000";
      condmet <= '0';
    elsif (SMCLK'event) and (SMCLK='1') then  
      counter <= counter + '1'; 
      if (counter(7 downto 0)="11111111") then
        refresh <= '1';       
      else 
        refresh <= '0';
      end if;
      -- Presla dostatocna doba pre aktivaciu druheho pismena
      -- Pouzivam 2^22 namiesto presnych 4000000 taktov pre zefektivnenie obvodu
      if(counter(22)='1') then
        condmet <= '1';
      end if;
    end if;
  end process;
        
           
  -- Posuvny register            
  process(RESET, SMCLK, refresh) is
  begin
    if (RESET='1') then
      led_content <= "11111110";
    elsif (SMCLK'event and SMCLK='1' and refresh='1') then
      -- Posuvam riadok  
      led_content <= led_content(6 downto 0) & led_content(7);
    end if;
  end process;            
        
              
  -- Dekoder
  process(RESET, led_content, condmet) is
  begin
    if (RESET='1') then
      row_content <= "00000000";
    elsif(condmet='1') then
      case led_content is
        -- Vypisujem oba znaky, kedze bola splnena casova podmienka
        when "01111111" => row_content <= "01010000";
        when "10111111" => row_content <= "10101000";
        when "11011111" => row_content <= "10101000";
        when "11101111" => row_content <= "11111000";
        when "11110111" => row_content <= "00111110";
        when "11111011" => row_content <= "00001001";
        when "11111101" => row_content <= "00001001";
        when "11111110" => row_content <= "00111110";
        when others => row_content <= "00000000";
      end case;
    else
      -- Vypisujem len Acko, teda T<0.5s
      case led_content is                            
        when "11110111" => row_content <= "00111110";
        when "11111011" => row_content <= "00001001";
        when "11111101" => row_content <= "00001001";
        when "11111110" => row_content <= "00111110";
        when others => row_content <= "00000000";
      end case;
    end if;
  end process;
  
  
  -- Vystup signalov        
  LED <= led_content;
  ROW <= row_content;
              
end behv;
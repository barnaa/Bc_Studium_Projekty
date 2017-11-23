#!/bin/sh

# test.sh pre projekt 2 do IPK
# Vyrobil Andrej Barna (xbarna01)
echo "#1: Preklad serveru a klienta:"
make
echo ""
echo "#2: Spustenie serveru na porte 48648:"
./server -p 48648 &
IPK2SERVER_PID=$!
echo "  PID procesu serveru: $IPK2SERVER_PID"
echo ""
echo "#3: Test, ci nenastava chyba pri komunikacii ak su server aj klient v rovnakom priecinku - prenasany subor by mal zostat neposkodeny."
echo ""
echo "  #3.1: Stiahnutie suboru klientom:"
(./client -h localhost -p 48648 -d textfile.txt && echo "    [OK] ") || echo "    !CHYBA!"        
echo ""
echo "  #3.2: Nahranie suboru klientom na server:"
(./client -h localhost -p 48648 -u textfile.txt && echo "    [OK] ") || echo "    !CHYBA!"
echo ""
#echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "#4: Test beznej funkcie klienta aj serveru."
echo "  #4.1: Tvorba priecinku clientFolder a premiestnenie klienta a ukazkovych suborov (loremipsum.txt, mapa.png, badum_tss.wav) do daneho priecinku:"
mkdir clientFolder
mv client clientFolder
mv loremipsum.txt clientFolder
mv mapa.png clientFolder
mv badum_tss.wav clientFolder
echo "    Koniec presuvania suborov."
echo ""
echo "  #4.2: Stiahnutie suborov klientom:"
cd clientFolder                                                                            
echo "    - textfile.txt:"
(./client -h localhost -p 48648 -d textfile.txt && echo "      [OK] ") || echo "      !CHYBA!"
echo "    - foto.jpg:"
(./client -h localhost -p 48648 -d foto.jpg && echo "      [OK] ") || echo "      !CHYBA!"            
echo "    - trombone.wav:"
(./client -h localhost -p 48648 -d trombone.wav && echo "      [OK] ") || echo "      !CHYBA!"
echo ""
echo "  #4.3: Nahratie suborov klientom na server:"
echo "    - loremipsum.txt:"
(./client -h localhost -p 48648 -u loremipsum.txt && echo "      [OK]") || echo "      !CHYBA!"      
echo "    - mapa.png:"
(./client -h localhost -p 48648 -u mapa.png && echo "      [OK]") || echo "      !CHYBA!"        
echo "    - badum_tss.wav:"
(./client -h localhost -p 48648 -u badum_tss.wav && echo "      [OK] ") || echo "      !CHYBA!"
cd ..
echo ""
echo "#5: Vypinam server"
kill $IPK2SERVER_PID

/* IMP Projekt: Interaktivne svetelne noviny na FITkite
 * Autor: Andrej Barna (xbarna01)
 * Semester: zimny, ak. rok: 2016/2017
 * Podiel na subore: original
 * Datum poslednej upravy: 11.12.2016
 */


#include <stdio.h>
#include <string.h>
#include <fitkitlib.h>
#include <lcd/display.h>
#include <keyboard/keyboard.h>


///////////////////////////////////////////////////////////////////////////////
// Definicie konstant pre ovladanie funkcnosti
///////////////////////////////////////////////

// Maximalna dlzka ulozenej spravy, pole kam sa tato sprava ulozi bude o bajt
// vacsie - koncova nula
#ifndef MESSAGESIZE
 #define MESSAGESIZE 512
#endif

// Timeout na stlacanie tlacidla, nie su to presne ms, ale kedze presne casy
// nie su pre tuto aplikaciu nutne, tak je to dostacujuce
#ifndef PRESSTIMEOUT
 #define PRESSTIMEOUT (750)
#endif


///////////////////////////////////////////////////////////////////////////////
// Globalne premenne a riadiace struktury
//////////////////////////////////////////

// Pole, do ktoreho sa bude ukladat sprava na vypis
char message[MESSAGESIZE+1] = {0, };

// Key Character Count - pocet moznych znakov pre kazdu klavesu, nepocitajuc cislo
const char kcc[10] = {2, 10, 3, 3, 3, 3, 3, 4, 3, 4};

// Key One Symbols -- pocet prvkov v poli sa musi rovnat kcc[1]
// Podobne by sa dalo urobit vsetky symboly, ale tak skoda miesta v pamati
const char kosyms[] = {'.', ',', '?', '!', '-', '(', ')', '/', '\'', '"'};

// Typing Control - uchovava informacie o stave pisania
struct {
  unsigned int writing:1; // TRUE ak sa zapisuje, FALSE pre vypis
  unsigned int caps:1;    // TRUE ak pri zapise je aktivny CAPS lock
  unsigned int held:1;    // flag zabranujuci vypisovaniu dalsich cisel po prvom ak sa drzi klavesa
  unsigned int texp:1;    // Time Expired flag - po uplynuti casu posunie kurzor, zabranuje samovolnemu posunu kurzoru
  unsigned int symc:4;    // Symbol Counter - uchovava ktory znak v poradi na klavese bol naposledy vypisany
} typec;
#define WRITING (typec.writing)
#define CAPS    (typec.caps)
#define HELD    (typec.held)
#define TEXP    (typec.texp)
#define SYMC    (typec.symc)

// Viewing Control - uchovava informacie o stave zobrazovania
struct {
  unsigned int paging:1;    // Zobrazovanie po strankach
  unsigned int scrolling:1; // Zobrazovanie po riadkoch
  unsigned int rotating:1;  // Posun textu
  unsigned int flashing:1;  // Blikanie ON/OFF
  unsigned int speed:4;     // Nasobitel zakladneho cakania (sluzi pre ovladanie rychlosti posunu textu)
} viewc;
#define PAGING    (viewc.paging)
#define SCROLLING (viewc.scrolling)
#define ROTATING  (viewc.rotating)
#define FLASHING  (viewc.flashing)
#define SPEED     (viewc.speed)

char lc;        // Posledna stlacena klavesa
char slc;       // Predposledna stlacena klavesa (nenastavuje sa na 0)
long cc;        // Character Cursor
long t;         // Cas
long tPressed;  // Cas posledneho stlacenia klavesy
long tReleased; // Cas posledneho uvolnenia klavesy


///////////////////////////////////////////////////////////////////////////////
// Nutne funkcie
/////////////////

void print_user_help(void){}

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){
  return CMD_UNKNOWN;
}

void fpga_initialized(){}

///////////////////////////////////////////////////////////////////////////////


// Na zaklade dekodovanej stlacenej klavesy (chKB) a stavu premennej typec
// vrati odpovedajuci znak, ktory ma byt vypisany, alebo nulu ak sa jednalo
// o kontrolnu klavesu
char kbToChar(char chKB){
  char c;
  char k = chKB - 0x30; // Cislo klavesy
  if(k > 9 || k < 0) // Osetrenie nenumerickych klaves
    return 0;

  if(SYMC == kcc[k]) // Symbol Counter je na poslednom indexe, tj. samotne cislo
    return k + 0x30;

  if(!k){ // Stlacena nula, vypise sa medzera alebo plus
    if(!SYMC)
      c = ' ';
    else
      c = '+';
  }
  else if(k == 1) // Stlacena jednotka, vypise sa specialny znak z pola
    c = kosyms[SYMC];
  else{ // Vypise sa pismeno na zaklade stlacenej klavesy a ci je CAPS zapnuty
    c = CAPS? 0x41 : 0x61;
    c += (k-2)*3 + (k>7 ? 1 : 0) + SYMC;
  }

  return c;
}


// Oprava kurzoru pri pisani, vzhladom na to ze LCD_write_string rozhadzuje
// kurzor a teda pri pisani nie je viditelny
// Inspirovane funkciou LCD_append_char z lcd/display.c
void curFix(){
  if (cc%LCD_CHAR_COUNT > (LCD_CHAR_COUNT/2 - 1))
    LCD_send_cmd(LCD_SET_DDRAM_ADDR | (LCD_SECOND_HALF_OFS + cc%LCD_CHAR_COUNT
                 - (LCD_CHAR_COUNT/2)), 0);
  else
    LCD_send_cmd(LCD_SET_DDRAM_ADDR | cc%LCD_CHAR_COUNT, 0);
}


// Spracovanie stlacania klaves
int keyboard_idle(){
  // Ziskanie stlacenej klavesy
  char c = key_decode(read_word_keyboard_4x4());

  if(!c){ // Ziadna klavesa nie je stlacena
    if(c != lc){ // Predtym bola stlacena nejaka klavesa
      slc = lc;
      lc = c;
      tReleased = t;
    }
    else if(t - tReleased >= PRESSTIMEOUT && !TEXP && slc >= '0' && slc <= '9'
            && cc < MESSAGESIZE){
      // Posun kurzoru ak vyprsal cas na editaciu posledne stlaceneho znaku
      cc++;
      LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON |
                   LCD_BLINKING_ON, 0);
      TEXP = 1;
    }
  }
  else if (c == lc){
    // Spracovanie drzania klavesy a zabranenie dalsiemu vypisu toho isteho cisla
    if(WRITING && t - tPressed > PRESSTIMEOUT * 2 && !HELD){
      HELD = 1;
      if(kbToChar(c))
        message[cc] = c;
    }
  }
  else{
    tPressed = t;
    if(WRITING){
      // Opakovane sa stlaca ta ista klavesa, zvysujeme Symbol Counter
      if(slc == c && !lc && !HELD && tPressed - tReleased < PRESSTIMEOUT)
        SYMC++;
      else{
        // Podmienky opakovaneho stlacania klavesy neboli splnene, pise sa novy
        // znak
        SYMC = 0;
        if((cc || (!cc && message[cc])) && !TEXP && slc >= '0' && slc <= '9' &&
           lc != 'D' && cc < MESSAGESIZE && cc < strlen(message))
          cc++;
        LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON |
                     LCD_BLINKING_ON, 0);
      }

      // Pretocenie symbol counteru
      if(SYMC > kcc[c-0x30])
        SYMC = 0;

      // Resetovanie priznakov obmedzujucich vypis znakov a posun kurzoru
      HELD = 0;
      TEXP = 0;

      if(lc) // Do slc ulozime predoslu klavesu len ak to nebola nula
        slc = lc;
      lc = c;

      // Spracovanie kontrolnych klaves
      switch(c){
        case '*': // Prepnutie do zobrazovacieho rezimu
          WRITING = 0;
          LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF |
                       LCD_BLINKING_OFF, 0);
          set_led_d5(0);
          set_led_d6(0);
          return 0;
        case 'A': // Posun kurzoru spat
          if(cc)
            cc--;
          return 0;
        case 'B': // Posun kurzoru vpred
          if(cc != strlen(message) && cc < MESSAGESIZE)
            cc++;
          return 0;
        case 'C': // Vycistenie znaku, sprava sa ako backspace ak je na konci
          if(cc == strlen(message) && cc){
            message[cc-1] = 0;
            cc--;
          }
          else message[cc] = ' ';
          return 0;
        case 'D': // Zapnutie/vypnutie CAPS locku
          CAPS++;
          flip_led_d5();
          return 0;
        case '#':
          return 0;
        default:;
      }

      // Vypis znaku
      c = kbToChar(c);
      if(c)
        message[cc] = c;
      LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON |
                   LCD_BLINKING_OFF, 0);
    }
    else{ // Vypis textu
      // Zabranenie nasobneho prepinania klavesy (akoby bola viackrat stlacena)
      if(lc == c && t - tPressed < PRESSTIMEOUT / 5)
        return 0;

      // Spracovanie kontrolnych znakov
      switch(c){
        case '#': // Prepnutie do rezimu pisania
          WRITING = 1;
          LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_ON |
                       LCD_BLINKING_ON, 0);
          LCD_send_cmd(LCD_CURSOR_AT_HOME, 0);
          SYMC = 0;
          lc = 0;
          slc = 0;
          if(cc > strlen(message))
            cc = strlen(message);
          set_led_d6(1);
          if(CAPS)
            set_led_d5(1);
          return 0;
        case 'A': // Zapnutie/vypnutie rezimu po dvoch riadkoch (paging)
          PAGING++;
          SCROLLING = ROTATING = 0;
          break;
        case 'B': // Zapnutie/vypnutie rezimu po riadku (scrolling)
          SCROLLING++;
          PAGING = ROTATING = 0;
          break;
        case 'C': // Zapnutie/vypnutie rezimu rotovania (rotating)
          ROTATING++;
          SCROLLING = PAGING = 0;
          break;
        case 'D': // Zapnutie/vypnutie blikania
          FLASHING++;
          LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON, 0);
        case '*':
          break;
        default: // Nastavenie rychlosti
          SPEED = 10-c+0x30;
      }
      lc = c;
      tPressed = t;
    }
  }
  return 0;
}


// Funkcia pre vypis spravy na LCD v zavislosti od offsetu, od ktoreho sa ma
// vypisovat, obsahuje specianu upravu pre koniec spravy pri rotovani textu
void writeOnLCD(int offset){
  char screen[LCD_CHAR_COUNT];
  int fcr;
  if(ROTATING && offset > strlen(message)){
    fcr = offset - strlen(message);
    memset(screen, ' ', LCD_CHAR_COUNT);
    strncpy(screen + LCD_CHAR_COUNT - fcr, message,
            fcr > strlen(message) ? strlen(message) : fcr);
  }
  else strncpy(screen, message+offset, LCD_CHAR_COUNT);
  LCD_write_string(screen);
  if(WRITING)
    curFix();
}


// Funkcia main, obsahujuca inicializaciu pozadovanych prvkov a spracovanie
// klaves a vypis textu na LCD v cykle
int main(void){
  // Inicializacia premennych
  //char debugOut[32];
  int offset = 0;
  lc = slc = cc = t = tPressed = tReleased = 0;
  //*((char*)&typec) = *((char*)&viewc) = 0;
  memset(&typec, 0, 1);
  memset(&viewc, 0, 1);
  SPEED = 5;

  // Inicializacia hardware, klavesnice a LCD
  initialize_hardware();
  keyboard_init();
  LCD_init();
  LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);
  LCD_clear();

  // Vypnutie diod
  set_led_d5(0); // Caps
  set_led_d6(0); // Writing

  // Cyklus spracovania vstupu a vypisu na LCD
  while(1){
    delay_ms(1);
    t++;
    keyboard_idle(); // Vstup z klavesnice
    terminal_idle(); // Vstup z terminalu

    if(WRITING){ // Rezim zapisovania
      if(t%150 == 0)
        writeOnLCD(cc/LCD_CHAR_COUNT*2*16);
    }
    else{ // Rezim vypisu
      // Na vypnuty displej sa nevypisuje text korektne
      if(FLASHING)
        LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);

      // Nastavenie offsetu a vypis, writeOnLCD nie je na konci kvoli
      // rozdielnym intervalom nutnosti vypisat obsah LCD displeju
      if(PAGING){ // Posun po 2 riadkoch
        if(t%(2400*SPEED) == 0){
          offset = ((offset/LCD_CHAR_COUNT+1)*LCD_CHAR_COUNT)%
                    ((strlen(message)/LCD_CHAR_COUNT + 1)*LCD_CHAR_COUNT);
          writeOnLCD(offset);
        }
      }
      else if(SCROLLING){ // Posun po 1 riadku
        if(t%(1200*SPEED) == 0){
          offset = ((offset/16+1)*16)%((strlen(message)/16 + 1)*16);
          writeOnLCD(offset);
        }
      }
      else if(ROTATING){ // Posun po 1 znaku
        if(t%(75*SPEED) == 0){
          offset = (offset+1)%(strlen(message)+LCD_CHAR_COUNT);
          writeOnLCD(offset);
        }
      }

      if(FLASHING){ // Blikanie
        if(t/150%4)
          LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);
        else
          LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_OFF, 0);
      }
      else
        LCD_send_cmd(LCD_DISPLAY_ON_OFF | LCD_DISPLAY_ON | LCD_CURSOR_OFF, 0);
    }
    /* Test casovej jednotky t v porovnani s realnymi sekundami
    if(t%1000 == 0){
      sprintf(debugOut, "t: %ld", t);
      term_send_str_crlf(debugOut);
    }
    // 1s IRL ~ 0.9s tohto casu pri vypise bez zataze (strata 1/10, 60s~54)
    // 1s IRL ~ 0.84s tohto casu pri agresivnom zapisovani (strata 1/6 60s~50)
    */
  }
}

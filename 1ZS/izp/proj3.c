/* |=====================================| *
 * |    Projekt 3: Prechod bludiskom    | *
 * |    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    | *
 * |                                     | *
 * | Vypracoval: Andrej Barna <xbarna01> | *
 * | S�bor: proj3.c                      | *
 * | Ak. rok: 2014/2015                  | *
 * | Semester: prv�, zimn�               | *
 * |=====================================| */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define PMAXLEN 512 // Maximalna dlzka trasy pri hladani najkratsej trasy


// Struktura mapy dana zadanim
typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;


// Do tejto struktury sa uklada aktualna pozicia v mape
typedef struct {
  int row;
  int col;
  char dir;      // direction: vlavo (1), vpravo (2), vertikalne (4)
} Position;


/*
 * Enum pre ujasnenie stran v porovnavani
 * ROTL znamena, ze sa ma zmenit smer na prvy zlava, teda na VERT
 * ROTR znamena to iste pre opacny smer, teda sa smer ma zmenit na LEFT
 * ENDPOS je specialny smer, ktory je vyuzity pri hladani najkratsej trasy a
 *        signalizuje to, ze bol dosiahnuty koniec bludiska
 */
enum sides {ROTL=0, LEFT=1, RIGHT=2, VERT=4, ROTR=8, ENDPOS=64 };


// Chyby, ktore mozu nastat
enum errors
{
  EOK,
  ENOARGS,
  ENEARGS,
//ETMARGS,
  EILLARGS,
  EARGNAN,
  EISTART,
  EFNOPEN,
  EFDIMNF,
  EFDIMNV,
  EFMNALL,
  EFNVALD,
  EMNVALD,
  ENEXITF,
  EPATHTL
};


// Chybove hlasenia
const char *errmsg[] =
{
  [ENOARGS] = "Chyba! Nezadali ste argumenty! Spustenie s argumentom --help vytlaci napovedu.\n",
  [ENEARGS] = "Chyba! Nedostatocny pocet argumentov!\n",
//[ETMARGS] = "Chyba! Privela argumentov!\n",
  [EILLARGS] = "Chyba! Spustili ste program s nespravnymi argumentami!\n",
  [EARGNAN] = "Chyba! V argumente, ktory ma byt cislo, sa nachadza iny znak nez cislica!\n",
  [EISTART] = "Chyba! Vstup do bludiska je na neplatnej pozicii!\n",
  [EFNOPEN] = "Chyba! Subor bludiska sa nepodarilo otvorit!\n",
  [EFDIMNF] = "Chyba! V subore bludiska sa nenachadzaju rozmery na prvom riadku!\n",
  [EFDIMNV] = "Chyba! V subore bludiska su neplatne rozmery!\n",
  [EFMNALL] = "Chyba! Nepodarilo sa alokovat pamat pre mapu!\n",
  [EFNVALD] = "Chyba! V subore mapy sa nachadzaju neocakavane znaky!\n",
  [EMNVALD] = "Chyba! Mapa nie je validna!\n",
  [ENEXITF] = "Chyba! V mape nebol najdeny vychod!\n",
  [EPATHTL] = "Chyba! Vysledna trasa je pridlha! Skontrolujte prechodnost mapy.\n"
};


const char help[] =
"Projekt 3: Prechod bludiskom\n"
"Vypracoval: Andrej Barna (xbarna01)\n"
"Akademicky rok: 2014/2015\n\n"
"Program sluzi k najdeniu cesty z bludiska, ktore je zlozene z trojuholnikovych\n"
"buniek. Taktiez dokaze overit validitu bludiska daneho argumentom programu,\n"
"alebo najst najkratsiu cestu bludiskom.\n\n"
"Program sa da spustit s nasledujucimi argumentami:\n"
" --help - vypise tuto napovedu\n"
" --test nazov_suboru - overi validitu bludiska\n"
" --rpath X Y nazov_suboru - hlada cestu bludiskom metodou pravej ruky, teda sa\n"
"\tpri hladani cesty drzi steny napravo, X a Y urcuju riadok a stlpec\n"
"\tbunky, z ktorej sa bude hladat cesta. Bunka musi byt vstupom do bludiska\n"
" --lpath X Y nazov_suboru - hlada cestu bludiskom metodou lavej ruky, teda sa\n"
"\tpri hladani cesty drzi steny nalavo, X a Y urcuju riadok a stlpec bunky,\n"
"\tz ktorej sa bude hladat cesta. Bunka musi byt vstupom do bludiska\n"
" --shortest X Y nazov_suboru - vyhlada najkratsiu cestu bludiskom, pricom trasu\n"
"\tvyhladava od bunky danej cislami X a Y urcujucimi jej riadok a stlpec\n"
" nazov_suboru - textovy subor, v ktorom je ulozene bludisko\n";


////////////////////////////////////////////////////////////////////////////////
// Prototypy funkcii:
bool isborder(Map *map, int r, int c, int border);
int start_border(Map *map, int r, int c, int leftright);

bool processTest(Map *map);
bool processPath(Map *map, Position *pos, int leftright);
bool processShortest(Map *map, Position *pos);

bool loadMap(Map *map, char *fileName);
void destroyMap(Map *map);

bool testMap(Map *map);
void changeDir(Position *pos, int leftright);
bool onBoundLimit(Map *map, Position *pos);
int getExitCount(Map *map);
void move(Position *pos);
unsigned int pathfinding(Map *map, Position *pos, char *path, char *spath, unsigned int pathlen);
////////////////////////////////////////////////////////////////////////////////




/*
 * Vo funkcii main sa spracovavaju argumenty, ktore sa dalej predavaju funkciam
 * uz len pomocou struktur, respektive pomocou premennych
 */
int main(int argc, char *argv[])
{
////////////////////////////////////////////////////////////////////////////////
  if (argc == 1) // Bez argumentov
  {
    fprintf(stderr,errmsg[ENOARGS]);
    return EXIT_FAILURE;
  }
////////////////////////////////////////////////////////////////////////////////
  else if (!strcmp(argv[1],"--help")) // Vypis helpu
  {
    printf(help);
    return EXIT_SUCCESS;
  }
////////////////////////////////////////////////////////////////////////////////
  else if (!strcmp(argv[1],"--test")) // Overovanie validity bludiska
  {
    if (argc < 3)
    {
      fprintf(stderr,errmsg[ENEARGS]);
      return EXIT_FAILURE;
    }

    Map map;
    if (loadMap(&map, argv[2]))
      return EXIT_FAILURE;

    if (testMap(&map))
    {
      printf("Invalid\n");
    }
    else printf("Valid\n");
    destroyMap(&map);
    return EXIT_SUCCESS;
  }
////////////////////////////////////////////////////////////////////////////////
  else if (!strcmp(argv[1],"--rpath") || !strcmp(argv[1],"--lpath") || !strcmp(argv[1],"--shortest")) // Hladanie trasy
  {
    if (argc < 5)
    {
      fprintf(stderr,errmsg[ENEARGS]);
      return EXIT_FAILURE;
    }

    // Tvorba struktury pozicie, vkladanie argumentov do nej
    Position pos;
    char *end;
    pos.row = strtol(argv[2],&end,10);
    if (*end)
    {
      fprintf(stderr,errmsg[EARGNAN]);
      return EXIT_FAILURE;
    }
    pos.col = strtol(argv[3],&end,10);
    if (*end)
    {
      fprintf(stderr,errmsg[EARGNAN]);
      return EXIT_FAILURE;
    }

    // Nacitanie a overovanie mapy
    Map map;
    if (loadMap(&map, argv[4]))
      return EXIT_FAILURE;
    if (testMap(&map))
    {
      fprintf(stderr,errmsg[EMNVALD]);
      destroyMap(&map);
      return EXIT_FAILURE;
    }
    if (!getExitCount(&map))
    {
      fprintf(stderr,errmsg[ENEXITF]);
      destroyMap(&map);
      return EXIT_FAILURE;
    }

    // Overovanie, ci nie je pociatocna pozicia mimo bludiska
    if (onBoundLimit(&map, &pos))
    {
      destroyMap(&map);
      fprintf(stderr,errmsg[EISTART]);
      return EXIT_FAILURE;
    }

    // Ak je to hladanie najkratsej trasy
    if (!strcmp(argv[1],"--shortest"))
    {
      if (processShortest(&map, &pos))
      {
        destroyMap(&map);
        return EXIT_FAILURE;
      }
    }
    // Ak je to hladanie trasy pomocou pravidla pravej, resp. lavej ruky
    else
    {
      int leftright = 0; // Metoda pr./l. ruky, ak je 1 tak sa ide pravou r.
      if (!strcmp(argv[1],"--rpath"))
        leftright = 1;

      if (processPath(&map, &pos, leftright))
      {
        destroyMap(&map);
        return EXIT_FAILURE;
      }
    }

    // Zrusenie mapy a navrat do funkcie main
    destroyMap(&map);
    return EXIT_SUCCESS;
  }
////////////////////////////////////////////////////////////////////////////////
  else
  {
    fprintf(stderr,errmsg[EILLARGS]); // Nespravne argumenty
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


/*
 * Prebehne bludiskom, zaklad funkcnosti --rpath a --lpath
 * ak leftright == 1, tak sa ide rpath, inak lpath
 */
bool processPath(Map *map, Position *pos, int leftright)
{
  // Urcenie pociatocneho smeru a overovanie, ci je to platny vstup do bludiska
  pos->dir = start_border(map, pos->row, pos->col, leftright);
  if (!pos->dir)
  {
    fprintf(stderr,errmsg[EISTART]);
    return EXIT_FAILURE;
  }

  // Samotny prechod bludiskom, ktory prebieha, kym sa z neho nedostaneme von
  while(!onBoundLimit(map, pos))
  {
    if (isborder(map, pos->row, pos->col, pos->dir))
    {
      changeDir(pos, leftright);
    }
    else
    {
      printf("%d,%d\n", pos->row, pos->col);
      move(pos);
      if (pos->dir != VERT)
        pos->dir = (pos->dir == RIGHT)? LEFT : RIGHT;
        changeDir(pos, leftright);
    }
  }
  return true;
}


/*
 * Tato funkcia hlada najkratsiu trasu bludiskom, ktoru nasledne vypisuje
 * Trasa sa uklada do retazca spath
 */
bool processShortest(Map *map, Position *pos)
{
  pos->dir = ROTL;
  char path[PMAXLEN] = "";
  char spath[PMAXLEN] = "";
  pathfinding(map, pos, path, spath, 0);
  if (strlen(spath) >= PMAXLEN || !strlen(spath))
  {
    fprintf(stderr,errmsg[EPATHTL]);
    return EXIT_FAILURE;
  }
  for (unsigned int i = 0; isdigit(spath[i]+'0') && spath[i]; i++)
  {
    printf("%d,%d\n", pos->row, pos->col);
    pos->dir = (int) spath[i];
    move(pos);
  }
  return EXIT_SUCCESS;
}


// Nacita mapu zo suboru, overuje platnost hodnot v subore bludiska
bool loadMap(Map *map, char *fileName)
{
  FILE *subor = fopen(fileName,"r");
  if (subor == NULL)
  {
    fprintf(stderr,errmsg[EFNOPEN]);
    return EXIT_FAILURE;
  }
  if (fscanf(subor,"%d %d\n", &map->rows, &map->cols)!=2)
  {
    fprintf(stderr,errmsg[EFDIMNF]);
    fclose(subor);
    return EXIT_FAILURE;
  }
  if (map->rows < 1 || map->cols < 1)
  {
    fprintf(stderr,errmsg[EFDIMNV]);
    fclose(subor);
    return EXIT_FAILURE;
  }

  map->cells = malloc(map->cols*map->rows*sizeof(char));
  if (map->cells == NULL)
  {
    fprintf(stderr,errmsg[EFMNALL]);
    fclose(subor);
    return EXIT_FAILURE;
  }

  int c;
  bool chyba = false;
  for (int i = 0; i < map->rows; i++)
  {
    for (int j = 0; j < map->cols; j++)
    {
      while(isspace(c = fgetc(subor))){};
      if (c >= '0' && c <= '7')
        map->cells[i*map->cols+j] = c - '0';
      else
      {
        chyba = true;
        break;
      }
      c = fgetc(subor);
      if (!isspace(c))
      {
        chyba = true;
	      break;
      }
      if (c == EOF && (i!=map->rows-1 || j!=map->cols-1))
      {
        chyba = true;
	      break;
      }
    }
    if (chyba)
      break;
  }
  fclose(subor);
  if (chyba)
  {
    fprintf(stderr,errmsg[EFNVALD]);
    free(map->cells);
  }
  return chyba;
}


// Znici mapu
void destroyMap(Map *map)
{
	free(map->cells);
	memset(map, 0, sizeof(Map));
}


// Urci, ktorym smer sa ma na zaciatku ist
int start_border(Map *map, int r, int c, int leftright)
{
// leftright je v mojej interpretacii bool; true znamena, ze sa ide pravidlom
// pravej ruky
  if (r&1 && c==1 && !isborder(map, r, c, LEFT)) // r je neparne, prvy stlpec
    return leftright? RIGHT : VERT;
  else if (!(r&1) && c==1 && !isborder(map, r, c, LEFT)) // r je parne, prvy stlpec
    return leftright? VERT : RIGHT;
  else if (r==1 && c&1 && !isborder(map, r, c, VERT)) // vstup zhora
    return leftright? LEFT : RIGHT;
  else if (r == map->rows && (r+c)&1 && !isborder(map, r, c, VERT)) // vstup zdola
    return leftright? RIGHT : LEFT;
  else if ((c == map->cols) && !((r+c)&1) && !isborder(map, r, c, RIGHT)) // vstup sprava, bunka s vrchnou hranou
    return leftright? VERT : LEFT;
  else if ((c == map->cols) && (r+c)&1 && !isborder(map, r, c, RIGHT)) // vstup sprava, bunka so spodnou hranou
    return leftright? LEFT : VERT;
  else return 0;
}


// Zistuje, ci dana bunka obsahuje danu hranicu
// 1 je lava, 2 je prava a 4 je vertikalna hranica
bool isborder(Map *map, int r, int c, int border)
{
  if (!(map->cells[(r-1)*map->cols+(c-1)]&border))
    return false;
  else
    return true;
}


// Overuje validitu mapy
bool testMap(Map *map)
{
  for (int i = 1; i <= map->rows; i++)
  {
    for (int j = 1; j <= map->cols; j++)
    {
      if ((j+i)&1)
      {
        //Bunka smeruje hore (ma dolnu stenu) - neparny sucet
        if ((i!=map->rows) && (isborder(map,i,j,VERT)!=isborder(map,i+1,j,VERT)))
          return EXIT_FAILURE;
      }
      else
      {
        //Bunka smeruje dole (ma hornu stenu) - parny sucet
        if ((i != 1) && (isborder(map,i,j,VERT)!=isborder(map,i-1,j,VERT)))
          return EXIT_FAILURE;
      }
      if ((j != 1) && (isborder(map,i,j,LEFT)!=isborder(map,i,j-1,RIGHT)))
        return EXIT_FAILURE;
      if ((j!=map->cols) && (isborder(map,i,j,RIGHT)!=isborder(map,i,j+1,LEFT)))
        return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}


// Spocita "vychody" z mapy
int getExitCount(Map *map)
{
   // exc = Exit Count, teda pocet vychodov z mapy
  int exc = 0;

  for(int i = 3; i < map->cols; i+=2) // Prvy riadok
    if (!isborder(map, 1, i, VERT))
      exc++;
  for(int i = ((((map->rows)&1)==0)? 3: 2); i < map->cols; i+=2) // Posledny riadok
    if (!isborder(map, map->rows, i, VERT))
      exc++;
  for(int i = 2; i < map->rows; i++) // Prvy stlpec
    if (!isborder(map, i, 1, LEFT))
      exc++;
  for(int i = 2; i < map->rows; i++)// Posledny stlpec
    if (!isborder(map, i, map->cols, RIGHT))
      exc++;

  // Rohy
  if (!isborder(map, 1, 1, VERT) || !isborder(map, 1, 1, LEFT)) // Lavy  horny
    exc++;
  if (!isborder(map, 1, map->cols, RIGHT) || ((map->cols&1) // Pravy horny
      && (!isborder(map, 1, map->cols, VERT))))
    exc++;
  if (!isborder(map, map->rows, 1, LEFT) || (!(map->rows&1)
      && (!isborder(map, map->rows, 1, VERT)))) // Lavy dolny
    exc++;
  if (!isborder(map, map->rows, map->cols, RIGHT) || (((map->rows + map->cols)&1)
      && (!isborder(map, map->rows, map->cols, VERT)))) // Pravy dolny
    exc++;
  return exc;
}


/*
 *  Zistuje, ci sme sa dostali von z bludiska - vrati true, ak sme mimo mapy
 */
bool onBoundLimit(Map *map, Position *pos)
{
  if (pos->row < 1 || pos->row > map->rows)
    return true;
  if (pos->col < 1 || pos->col > map->cols)
    return true;
  return false;
}


/*
 * Zmeni aktualny smer podla typu prechodu a smeru
 */
void changeDir(Position *pos, int leftright)
{
    // trirot = Triangle rotation, true ak ma dolnu hranu, teda je bunka otocena
    //          dohora
  bool trirot = (pos->row + pos->col)&1;
  if (trirot == leftright) // Smer sa deli 2 (bitovym posunom vlavo)
  {
    pos->dir >>= 1;
    if (pos->dir == ROTL)
      pos->dir = VERT;
  }
  else // Smer sa nasobi 2 (bitovym posunom vpravo)
  {
    pos->dir <<= 1;
    if (pos->dir == ROTR)
      pos->dir = LEFT;
  }
  return;
}


/*
 * Zmeni poziciu v pos podla smeru pos->dir
 */
void move(Position *pos)
{
  switch(pos->dir)
  {
    case LEFT: // vlavo
      pos->col -= 1;
    break;
    case RIGHT: // vpravo
      pos->col += 1;
    break;
    case VERT: // hore/dole
      if ((pos->row + pos->col)&1)
        pos->row += 1;
      else
        pos->row -= 1;
    break;
    default:
    break;
  }
  return;
}


/*
 * Hladanie najkratsej trasy, riesene pomocou rekurzie. Parametre:
 *  Map *map - nemenna mapa bludiska
 *  Pos *pos - pozicia bunky, z ktorej sa vykonava telo funkcie
 *  char *path - aktualna trasa od startovacej pozicie
 *  char *spath - najkratsia konecna trasa
 *  unsigned int pathlen - aktualna dlzka trasy od startovacej pozicie
 *
 * Podmienky:
 *  - ak je dlzka aktualnej trasy vacsia, resp. rovna dlzke aktualnej najkratsej
 *    trasy, tak sa rekurzia prerusi
 *  - ak sme mimo bludiska a aktualna trasa je kratsia ako spath, tak ulozime do
 *    spath aktualnu trasu a vratime aktualnu dlzku trasy - inak sa len vratime
 *  - rekurzivne volame tuto funkciu pre pozicie:
 *      1. na ktorych sme neboli naposledy (tj. bunka, z ktorej sme prisli)
 *      2. na ktore sa da ist (nie je hranica medzi aktualnou a dalsou bunkou)
 *      -> funkciu mozeme rekurzivne volat max. pre 2 bunky, vynimkou je prve
 *         spustenie funkcie, ak by bola pociatocna bunka 0 tak by bol vynechany
 *         jeden smer, kde by mohla byt najkratsia trasa
 *
 * Tato funkcia potrebuje, aby sa po jej zavolani este vypisala trasa, ktoru
 * vracia cez spath metodou smerov, ktorymi treba ist - to je najkratsia trasa
 */
unsigned int pathfinding(Map *map, Position *pos, char *path, char *spath, unsigned int pathlen)
{
  if (pathlen >= PMAXLEN)
    return PMAXLEN;
  if (pathlen >= strlen(spath) && strlen(spath)) // Pridlha trasa
    return UINT_MAX;
  if (onBoundLimit(map, pos)) // Koniec cesty, bunka je mimo bludiska
  {
    if (!strlen(spath) || strlen(spath)>pathlen)
      strcpy(spath, path);
    pos->dir = ENDPOS;
    return pathlen;
  }

  char oldpath[pathlen];
  strcpy(oldpath, path);

  Position newpos;
  newpos.dir = pos->dir;
  if (newpos.dir!=VERT && newpos.dir != ROTL)
    newpos.dir = (newpos.dir&1)? RIGHT : LEFT;

  unsigned int newlen = UINT_MAX;
  unsigned int prot = 2;

  // Zvysi pocet rotacii pri prvom spusteni funkcie, aby sa presli vsetky smery
  if (!newpos.dir)
  {
    prot++;
    newpos.dir = VERT;
  }

  // Testuje, ci sa da ist dalsimi 2 smermi (3, ak to je prve spustenie funkcie)
  for (unsigned int i = 0; i < prot; i++)
  {
    newpos.row = pos->row;
    newpos.col = pos->col;
    changeDir(&newpos,1);
    char newpath[strlen(oldpath)+1];
    strcpy(newpath, oldpath);

    if (!isborder(map, newpos.row, newpos.col, newpos.dir)) // Da sa ist dalej
    {
      move(&newpos);
      char newpath[strlen(oldpath)+1];
      strcpy(newpath, oldpath);
      newpath[pathlen] = newpos.dir; // Path obsahuje netlacitelne znaky 1, 2, 4
      newlen = pathfinding(map, &newpos, newpath, spath, pathlen+1); // Rekurzia
    }
    else newlen = UINT_MAX; // Ak je to stena, tak vracia, ze trasa je pridlha
    if (newpos.dir == ENDPOS) // Nema zmysel ist dalej ak je koniec rovno pred nosom
      break;
  }
  return newlen;
}

/* |======================================| *
 * |     Projekt 1: Vypocty v tabulke     | *
 * |     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~     | *
 * |                                      | *
 * | Vypracoval: Andrej Barna             | *
 * | Ak. rok: 2014/2015                   | *
 * | Semester: prvý, zimný                | *
 * |======================================| */


#include <stdio.h> // vstup, vystup
#include <stdlib.h> // atoi, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // strcmp, strlen
#include <ctype.h> // tolower, isspace
#include <limits.h> // INT_MAX


//enumy pre prehladnost kodu pri porovnavani podmienok
enum operacia {SELECT, MIN, MAX, SUM, AVG};
enum vyber {ROW, ROWS, COL, COLS, RANGE};

// Chyby, ktore mozu nastat
enum errors
{
  ENOARGS,
  EFEWARGS,
  EMANYARGS,
  EFARGL,
  EILLOP,
  EILLSEL,
  EILLSELRAN,
  ERTL,
  ENAN,
  ENEC,
  ENER,
  ETTB
};


// Chybove hlasenia
const char *errmsg[] =
{
  [ENOARGS] = "Chyba! Nezadali ste argumenty! Spustenie s argumentom --help vytlaci napovedu.\n",
  [EFEWARGS] = "Chyba! Nedostatocny pocet argumentov!\n",
  [EMANYARGS] = "Chyba! Privela argumentov!\n",
  [EFARGL] = "Chyba! Prvy ciselny argument pre vyber je vacsi nez druhy!\n",
  [EILLOP] = "Chyba! Zadali ste neplatnu operaciu!\n",
  [EILLSEL] = "Chyba! Zadali ste neplatny typ vyberu!\n",
  [EILLSELRAN] = "Chyba! Neplatny vyber operacie!\n",
  [ERTL] = "Chyba! Privela znakov na riadok - viac ako 1024!\n",
  [ENAN] = "Chyba! Funkcia, ktora ocakava ciselny vstup dostala iny znak nez cislo!\n",
  [ENEC] = "Chyba! V riadku sa nachadza menej stlpcov, nez pozaduje vyber!\n",
  [ENER] = "Chyba! V tabulke sa nachadza menej riadkov, nez pozaduje vyber!\n",
  [ETTB] = "Chyba! Prekrocili ste maximalny pocet riadkov tabulky!\n"
};


void help()
{
    printf("          Napoveda           \n"
           "          ~~~~~~~~           \n"
           "Program sa spusta ako:\n"
           " proj1.exe operacia typVyberu ciselneParametre\n\n"
           "Operacia:\n"
           " MIN - vytlaci minimum z hodnot\n"
           " MAX - vytlaci maximum z hodnot\n"
           " SUM - spocita obsah buniek\n"
           " AVG - vypocita aritmeticky priemer obsahu buniek\n"
           " SELECT - vytlaci obsah buniek na obrazovku\n\n"
           "Typ vyberu a prislusne ciselne parametre:\n"
           " ROW X - spracuje riadok oznaceny cislom x\n"
           " ROWS X Y - spracuje riadky od cisla x do cisla y vratane nich\n"
           " COL X - spracuje stlpec oznaceny cislom x\n"
           " COLS X Y - spracuje stlpce od cisla x do cisla y vratane nich\n"
           " RANGE A B X Y - spracuje bunky, ktore sa nachadzaju v rozsahu\n"
           "                 riadkov od A do B a sucasne v rozsahu stlpcov\n"
           "                 od X do Y vratane nich\n\n"
           "Operacie MIN, MAX, SUM a AVG dokazu pracovat len nad bunkami\n"
           "obsahujucimi ciselne udaje.\n");

}


/* Skontroluje, ci dlzka riadku nepresiahla 1024 znakov a ak ano,
 * tak vrati chybovy stav.
 */
int rowLengthCheck(int *rowLen)
{
  (*rowLen)++;
  if (*rowLen>1024)
  {
      fprintf(stderr,errmsg[ERTL]);
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


/* Spracuje prve 2 argumenty, s ktorymi bol program spusteny,
 * teda operaciu a druh vyberu a potom skontroluje, ci sedi pocet
 * argumentov pre pozadovany vyber.
 */
int spracujSlovneArgumenty(int argc, char *argv[], int *opNo, int 
*selNo)
{
  if (argc == 1)
  {
    fprintf(stderr, errmsg[ENOARGS]);
    return EXIT_FAILURE;
  }
  else if (strcmp(argv[1], "--help") == 0)
  {
    help();
    return (-1);
  }

  char arg1buffer[(strlen(argv[1]) < 8) ? strlen(argv[1]) : 8];
  (void) strcpy(arg1buffer, argv[1]);
  for (unsigned int i = 0; i < (strlen(arg1buffer)); i++)
    arg1buffer[i] = tolower(arg1buffer[i]);
  if(strcmp(arg1buffer, "select")==0) *opNo=SELECT;
  else if(strcmp(arg1buffer, "min")==0) *opNo=MIN;
  else if(strcmp(arg1buffer, "max")==0) *opNo=MAX;
  else if(strcmp(arg1buffer, "sum")==0) *opNo=SUM;
  else if(strcmp(arg1buffer, "avg")==0) *opNo=AVG;
  else
  {
    fprintf(stderr,errmsg[EILLOP]);
    return EXIT_FAILURE;
  }

  char arg2buffer[(strlen(argv[2]) < 8) ? strlen(argv[2]) : 8];
  (void) strcpy(arg2buffer,argv[2]);
  for (unsigned int i = 0; i < (strlen(arg2buffer)); i++)
    arg2buffer[i] = tolower(arg2buffer[i]);
  if(strcmp(arg2buffer, "row")==0) *selNo=ROW;
  else if(strcmp(arg2buffer, "rows")==0) *selNo=ROWS;
  else if(strcmp(arg2buffer, "col")==0) *selNo=COL;
  else if(strcmp(arg2buffer, "cols")==0) *selNo=COLS;
  else if(strcmp(arg2buffer, "range")==0) *selNo=RANGE;
  else
  {
    fprintf(stderr,errmsg[EILLSEL]);
    return EXIT_FAILURE;
  }
  

  int argNum = 4 + ((*selNo)&1); // ROWS alebo COLS
  argNum += (*selNo == RANGE)? 3 : 0; // pre RANGE
  if (argNum > argc)
  {
    fprintf(stderr,errmsg[EFEWARGS]);
    return EXIT_FAILURE; //primalo argumentov
  }
  else if (argNum < argc)
  {
    fprintf(stderr,errmsg[EMANYARGS]);
    return EXIT_FAILURE; //privela argumentov
  }
  return EXIT_SUCCESS;
}


/* Vypocita si pocet ciselnych argumentov pre prislusne typy vyberov a
 * spracuje ich, neskor preformatuje tieto argumenty a typ vyberu, co
 * je popisane nizsie.
 */
int spracujCiselneArgumenty(char *argv[], int *selNo, int *numArgs)
{
  int numArgc = 1;
  if (*selNo == RANGE)
    numArgc += 3;
  else if (*selNo == COLS || *selNo == ROWS)
    numArgc += 1;

  for (int i=0; i<numArgc; i++)
  {
    numArgs[i] = atoi(argv[i+3]);
    if (numArgs[i] < 1)
    {
      fprintf(stderr,errmsg[EILLSELRAN]);
      return EXIT_FAILURE;
    }
  }

/* ROW a COL su vlastne specialne pripady vyberov ROWS a COLS s rovnakymi
 * parametrami, teda to nahradzam pre zjednodusovanie podmienok neskor;
 * vyber je v poli numArgs ukladany ako keby to bol druh vyberu RANGE, teda
 * A B X Y, riadky A-B, stlpce X-Y
 */
  if ((*selNo == ROW) || (*selNo == COL))
  {
    numArgs[1] = numArgs[0];
    (*selNo)++;
  }
  if (*selNo == COLS)
  {
      for (int i = 0; i<2; i++) // velmi uzitocny cyklus, ale whatever
      {
          numArgs[i+2] = numArgs[i];
          numArgs[i] = -1; // pociatocna hodnota
      }
  }
  if (( numArgs[0] > numArgs[1] ) || ( numArgs[2] > numArgs[3] ))
  {
    fprintf(stderr,errmsg[EFARGL]);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


// Jednoducha kontrola, ci sa ma bunka spracovat alebo nie.
int patriDoVyberu(int selNo, int indexStlpca, int indexRiadku, int 
*numArgs)
{
  if (selNo != COLS) //teda rows alebo range
  {
    if ((indexRiadku < numArgs[0]) || (indexRiadku > numArgs[1]))
      return 0;
  }
  if (selNo != ROWS) //teda cols alebo range
  {
    if ((indexStlpca < numArgs[2]) || (indexStlpca > numArgs[3]))
      return 0;
  }
  return 1;
}


/* Funkcia vyuzivana len pri operacii SELECT, vycisti buffer, do ktoreho sa
 * uklada cislo pri spracovani bunky - teda povazuje kazdu bunku za cislo,
 * kym si program nie je isty, ze to nie je cislo - v tom pripade vypise cisla,
 * ktore boli ulozene v bufferi na standardny vystup, akoby to bol bezny text.
 */
void vycistenieBufferu(int *buffer, int *zeroc, int *isNegative, int *isNumber)
{
  if (*isNegative)
  {
    printf("-");
    *isNegative = 0;
  }
  if (*zeroc != 0)
    {
      for (int i = 0; i<*zeroc; i++)
      {
        printf("0");
      }
      *zeroc = 0;
    }
  if (*buffer != 0)
  {
    printf("%d",*buffer);
    *buffer = 0;
  }
  *isNumber = 0;
  return;
}


/* Ak bunka patri do vyberu, tak tato funkcia ju nacitava znak po znaku
 * a nasledne ju spracovava - v pripade operacie SELECT ju hned vypise,
 * v pripade operacii AVG, SUM hodnotu bunky pripocita do vyslData[0] a
 * pre operacie MIN, MAX priradi do vyslData[0] nizsiu, resp. vyssiu
 * z hodnot buffer a vyslData[0]. vyslData[1] je pocitadlo, ktore sa vyuziva
 * pre funkciu AVG po spracovani tabulky.
 */
int spracujBunku(int opNo, int znak, int *vyslData, int *rowlen)
{
  int buffer = 0, isNumber = 1, isNegative = 0;
  int zeroc = 0;  //spocitava nuly pred cislom pri operacii select
  do
  {
    if (rowLengthCheck(rowlen))
      return EXIT_FAILURE; // kedze na konci nacita znak, tak nespravi nic zle
    if (isNumber && znak >= '0' && znak <= '9')
    {
      buffer = (buffer*10)+(znak-'0');
      if (buffer == 0 && opNo == SELECT)
        zeroc++;
    }
    else if (isNumber && znak == '-')
    {
      if (buffer == 0 && isNegative == 0)
      {
        if (opNo == SELECT && zeroc != 0)
          vycistenieBufferu(&buffer, &zeroc, &isNegative, &isNumber);
        else
          isNegative = 1;
      }
      else
        if (opNo != SELECT)
        {
          fprintf(stderr,errmsg[ENAN]);
          return EXIT_FAILURE;
        }
        vycistenieBufferu(&buffer, &zeroc, &isNegative, &isNumber);
        putchar(znak);
    }
    else
    {
      if (opNo != SELECT)
      {
        fprintf(stderr,errmsg[ENAN]);
        return EXIT_FAILURE;
      }
      if (isNumber)
        vycistenieBufferu(&buffer, &zeroc, &isNegative, &isNumber);
      putchar(znak);
    }
  } while (!isspace(znak = getchar()) && znak != EOF);
  vyslData[1]++;
  if (isNumber)
  {
    buffer = (isNegative) ? -buffer : buffer;
  }
  if (opNo == MIN || opNo == MAX)
  {
    if (vyslData[1] == 1)
      vyslData[0] = buffer;
    else
    {
      if (opNo == MIN)
        vyslData[0] = vyslData[0] < buffer ? vyslData[0] : buffer;
      else
        vyslData[0] = vyslData[0] > buffer ? vyslData[0] : buffer;
    }
  }
  else if (opNo == SUM || opNo == AVG)
    vyslData[0] = vyslData[0]+buffer;
  else if (isNumber)
    printf("%.10g\n", (double) buffer);
  else printf("\n"); //toto robi ak je opNo SELECT
  return znak;
}


/* Nacitava znak po znaku zo standardneho vstupu, pricom ak znak nie je
 * biely tak prevedie kontrolu, ci patri do vyberu. Ak patri do vyberu,
 * tak zavola funckiu spracujBunku, inak nacita tolko znakov, kym nedojde
 * na biely znak, tj. preskoci bunku. Ak dojde na znak konca riadku, tak
 * jednoducho zvysi premennu indexRiadku a nastavi premenne indexStlpca a
 * rowlen povodne hodnoty. Po tom, ako sa dostane na koniec tabulky prevedie
 * finalne spracovanie a vypise vysledok na standardny vystup.
 */
int spracujTabulku(int opNo, int selNo, int *numArgs)
{
  int znak = 0;
  int indexStlpca = 1, indexRiadku = 1;
  int vyslData[2] = {0,0}; // vysledkova hodnota, counter
  int rowlen = 0; // premenna na dlzku riadku

  while ((znak = getchar()) != EOF)
  {
    if (rowLengthCheck(&rowlen))
      return EXIT_FAILURE;
    if (!isspace(znak))
    {
      if (patriDoVyberu(selNo, indexStlpca, indexRiadku, numArgs))
      {
        if ((znak = spracujBunku(opNo, znak, vyslData, &rowlen)) == 1)
          return EXIT_FAILURE;
      }
      else
        do
        {
          znak = getchar();
          if (rowLengthCheck(&rowlen))
            return EXIT_FAILURE;
        }
        while (!isspace(znak));
      indexStlpca++;
    }
    if (znak == '\n')
    {
      if (indexStlpca-1 < numArgs[3])
      {
        if (selNo == RANGE && (indexRiadku >= numArgs[0] && indexRiadku <= numArgs[1]))
        {
          fprintf(stderr, errmsg[ENEC]);
          return EXIT_FAILURE;
        }
      }
      if (indexRiadku == INT_MAX)
      {
        fprintf(stderr,errmsg[ETTB]);
        return EXIT_FAILURE;
      }
      rowlen = 0;
      indexStlpca=1;
      indexRiadku++;
    }
    if (znak == EOF) // poistka, keby getchar zo spracovania bunky nacital EOF
      break;
  }

  if (selNo != COLS)
    if (indexRiadku < numArgs[1])
    {
      fprintf(stderr,errmsg[ENER]);
      return EXIT_FAILURE;
    }
  if (opNo == AVG)
    printf("%.10g\n",(double) vyslData[0]/vyslData[1]);
  else if (opNo != SELECT)
  {
    printf("%.10g\n",(double) vyslData[0]);
  }
  return EXIT_SUCCESS;
}


/* Funkcia main je vyuzita len pre deklaraciu a inicializaciu zakladnych
 * premennych potrebnych pre chod programu a pre spravne fungovanie funkcii,
 * nasledne uz len volaju funkcie pre spracovanie argumentov a spracovanie
 * tabulky zo standardneho vstupu.
 */
int main(int argc, char * argv[])
{
  //premenne operationNumber, selectionNumber
  int opNo = 0, selNo = 0;

  //numericArguments - pole, v ktorom budu obsiahnute hodnoty a,b,x,y
  int numArgs[4] = {-1,-1,-1,-1};

  int i = spracujSlovneArgumenty(argc, argv, &opNo, &selNo);
  if (i == (-1))
    return EXIT_SUCCESS;
  else if (i)
    return EXIT_FAILURE;
  if (spracujCiselneArgumenty(argv, &selNo, numArgs))
    return EXIT_FAILURE;
  if (spracujTabulku(opNo, selNo, numArgs))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
/* |=====================================| *
 * |     Projekt 2: Iteracne vypocty     | *
 * |     ~~~~~~~~~~~~~~~~~~~~~~~~~~~     | *
 * |                                     | *
 * | Vypracoval: Andrej Barna <xbarna01> | *
 * | Súbor: proj2.c                      | * 
 * | Ak. rok: 2014/2015                  | *
 * | Semester: prvý, zimný               | *
 * |=====================================| */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


// Chyby, ktore mozu nastat
enum errors
{
  EOK,
  ENOARGS,
  ENEARGS,
//ETMARGS,
  EFARGL,
  EILLARGS,
  EARGNAN,
  EILLRAN,
  EILLANG,
  EILLMHT,
  EANGNST
};


// Chybove hlasenia
const char *errmsg[] =
{
  [ENOARGS] = "Chyba! Nezadali ste argumenty! Spustenie s argumentom --help vytlaci napovedu.\n",
  [ENEARGS] = "Chyba! Nedostatocny pocet argumentov!\n",
//[ETMARGS] = "Chyba! Privela argumentov!\n",
  [EFARGL] = "Chyba! Prvy ciselny argument pre vyber rozsahu je vacsi nez druhy!\n",
  [EILLARGS] = "Chyba! Spustili ste program s nespravnymi argumentami!\n",
  [EARGNAN] = "Chyba! V argumente, ktory ma byt cislo, sa nachadza iny znak nez cislica!\n",
  [EILLRAN] = "Chyba! Vyber iteracii musi byt v intervale <1;13>!\n",
  [EILLANG] = "Chyba! Uhol bol zadany mimo legalneho rozsahu (0;1.4>!\n",
  [EILLMHT] = "Chyba! Vyska meracieho pristroja musi byt v intervale (0;100>!\n",
  [EANGNST] = "Chyba! Uhol nebol urceny!\n"
};


const char help[] = 
"Projekt 2: Iteracne vypocty\n"
"Vypracoval: Andrej Barna (xbarna01)\n"
"Akademicky rok: 2014/2015\n\n"
"Program sluzi k vypoctu vzdialenosti a vysky objektu pomocou uhlov, pod \n"
"ktorymi vidno jeho zakladnu a vrchol. Tiez dokaze porovnat presnost vypoctu\n"
"tangens pomocou Taylorovho polynomu a zretazeneho zlomku.\n\n"
"Pouzitie pre porovnanie presnosti vypoctu tangens: ./proj2 --tan A N M\n"
" A je uhol v radianoch, pre ktory sa ma tangens pocitat (0 < A <= 1.4); N a\n"
" M urcuju interval, ktory definuje pre aky pocet iteracii sa ma porovnavat\n"
" presnost vypoctu tangens (0 < N <= M < 14).\n"
"Pouzitie pre vypocet vysky a vzdialenosti: ./proj2 [-c X] -m A [B]\n"
" Poznamka: Argumenty v hranatych zatvorkach su volitelne."
" Ak je dany argument -c tak sa nastavi vyska meracieho pristroja podla X\n"
" (0 < X <= 100). A a B su uhly alfa a beta, pricom pre obe plati, ze patria\n"
" intervalu (0.0,1.4>. Ak uhol beta nie je urceny tak sa pocita len \n"
" vzdialenost, inak sa pocita aj vyska.\n";


// Vrati absolutnu hodnotu x typu double
double absd(double x)
{
  return (x < 0.0)? -x : x;
}


// Vypocita tangens metodou pomocou Taylorovho polynomu
double taylor_tan(double x, unsigned int n)
{
  // Citatele a menovatele pre vypocet tangens Taylorovym polynomom
  const double tayCit[13] = {1, 1, 2, 17, 62, 1382, 21844, 929569, 6404582,
  443861162, 18888466084, 113927491862, 58870668456604};
  const double tayMen[13] = {1, 3, 15, 315, 2835, 155925, 6081075, 638512875,
  10854718875, 1856156927625, 194896477400625, 49308808782358125, 
  3698160658676859375};
  
  double pom = x;
  double medziVysl = 0;
  for (unsigned int i = 0.0; i < n; i++)
  {
    medziVysl += pom*(tayCit[i]/tayMen[i]);
    pom *= x*x;
  }
  return medziVysl;
}


// Vypocita tangens metodou zretazenych zlomkov
double cfrac_tan(double x, unsigned int n)
{
  double medzivys = n*2.0-1.0;
  for(; n-- > 1 ;)
    medzivys =(double) (n*2.0-1.0)-(x*x/medzivys);
  return x/medzivys;
}


// Zisti, kolko iteracii je potrebnych pre vypocitanie tangens
// pomocou zretazenych zlomkov s presnostou na 10 desatinnych miest
int getIterCount(double uhol)
{
//Ak je x vacsie alebo rovne prvku tak je potrebny pocet iteracii index_prvku+1
  const double iterationCount[10] = {0.0000000000, 0.0000000007, 0.0006694335,
  0.0214098345, 0.1065609063, 0.2759396556, 0.5205149192, 0.8149103472,
  1.1222490090, 1.3875327822};
//Poznamka: vynimky sa vyskytuju pri urceni uhlu s presnostou >7 des. miest, aj
//          to obvykle len o 1 iteraciu - o 2 iteracie menej staci jedine pre 
//          rozsah, kde su potrebne 3 iteracie

  int i = 9;
  for(; i-- >= 0 ;)
  {
    if (uhol >= iterationCount[i])
      return i+1;
  }
  return 10;
}


////////////////////////////////////////////////////////////////////////////////
/*Tento kod sluzi pre urcenie potrebneho poctu iteracii pre presny vypocet tan*/
/* na 10 desatinnych miest, bol pouzity pre urcenie iterationCount pola. Tato */
/* funckia postupne prechadza vsetky hodnoty na intervale <0;1.4> do hlbky    */
/* tolkych desatinnych miest, kolko urcuje parameter dm a nasledne skusa po   */
/* kolkych iteraciach sa dosiahne presnost tangens na 10 desatinnych miest pre*/
/* uhol urceny aktualnou hodnotou v danom intervale. Vysledne hodnoty vypisuje*/
/* na standardny vystup, vynimky vypisuje do suboru out.txt (ak je pritomny). */
/*Pouzitie: ./proj2 dm                                                        */   
/* -dm je pocet desatinnych miest, s ktorymi sa ma prepocitavat uhol beta, pri*/
/*     zadani vyssieho poctu des. miest prudko vzrasta narocnost programu     */
////////////////////////////////////////////////////////////////////////////////   
void iterationCounter(/*double cl,*/ int dm)
{
  double cl = 1.4; //ciselny limit
  double ho = 1.0; //hodnota old
  double hn = 0.0; //hodnota new
  int it = 1; //iteracny inkrement ran
  double ran = 0.0000000000; //udava, ktora hodnota sa testuje v tangens
  int j = 0; //pocitadlo iteracii
  int npi = 0; //najnizsi pocet iteracii, ktory je potrebny pre dosiahnutie presnosti na 10 des. miest
  FILE *log = fopen("out.txt","w");
  if (log == NULL)
    printf("UPOZORNENIE: Subor out.txt sa nepodarilo otvorit!\n");
  if (log)
    fprintf(log,"Format:\nPredpokladany vyzadovany pocet iteracii\tPocet iteracii, kedy bola dosiahnuta presnost\tUhol v rad\tTangens uhlu\tRozdiel medzi touto a predoslou iteraciou\n\n");
  for (int i = dm; i; i--)
    it *= 10;
  printf("Vysledky su zapisane vo nasledujucom formate:\n"
         "i\ttan(x,i)\ttan(x,i)-tan(x,i-1)\tx\n");
  for (int i = 0; i <= (double) (cl*it); i++)
  {
    do
    {
      j++;
      if(j>13)
        break;
      ho = hn;
      hn = cfrac_tan(ran, j);
    } while (absd(hn-ho) >= 0.0000000001);
    if (j > npi)
    {
      printf("%d\t %.10lf\t %.10lf\t %.10lf\n", j, hn, absd(hn-ho), ran);
      npi = j;
    }
    else if (j < npi && log != NULL)
      fprintf(log,"%d %d\t%.10lf\t%.10lf\t%.10lf\n", npi, j, ran, hn, absd(hn-ho));
    ran += (double) 1.0/it;
    j = 0;
  }
  if (log != NULL)
    fclose(log);
  return;                   
}
////////////////////////////////////////////////////////////////////////////////


// Ak sa program spusti s --tan tak sa zavola tato funkcia, ktora spracuje
// zvysne argumenty a nasledne vypise porovnanie hodnot podla zadania
int spracujTan(int argc, char *argv[])
{
  int err = 0; // Kod chyby
  if (argc < 5) 
  {
    fprintf(stderr,errmsg[ENEARGS]); // Nedostatok argumentov
    return EXIT_FAILURE;
  }
  char *end;
  double x = strtod(argv[2],&end);
  if (*end || isnan(x)) // Argument nie je cislo
    err = err? err : EARGNAN;
  if (x <= 0.0 || x > 1.4) // Uhol je mimo legalneho rozsahu
    err = err? err : EILLANG;
  unsigned int n = strtol(argv[3],&end, 10);
  if (*end) // Argument nie je cislo
    err = err? err : EARGNAN;
  unsigned int m = strtol(argv[4],&end, 10);
  if (*end) // Argument nie je cislo
    err = err? err : EARGNAN;
  if (n > m)
    err = err? err : EFARGL;                                   
  if(!(n>0) || !(m<14)) // Ilegalny rozsah
    err = err? err : EILLRAN;
    
  if (err) // Ak nastane jedna z chyb, tak ju vytlaci a ukonci program
  {
    fprintf(stderr,errmsg[err]);
    return EXIT_FAILURE;
  }
  
  double tanm,tay,cfrac; // Premenne pre: tan z kniznice math, tan vypocitany
         // cez Taylorov polynom a tan vypocitany pomocou zretazenych zlomkov
  for (unsigned int i = n; i <= m; i++)
  {
    tanm = tan(x);
    tay = taylor_tan(x,i);
    cfrac = cfrac_tan(x,i);
    printf("%d %e %e %e %e %e\n", i, tanm, tay, absd(tanm-tay),
                                  cfrac, absd(tanm-cfrac));
  }
  
  return EXIT_SUCCESS;
}


// Ak sa program spusti s -m alebo s -c tak sa zavola tato funkcia, ktora
// spracuje zvysne argumenty a nasledne vypise vzdialenost, resp. aj vysku, ak
// bol urceny uhol beta
int spracujMeranieVysky(int argc, char *argv[])
{   
  int currArg = 1; // sluzi pre urcenie pozicie argumentu, ktory sa ma spracovat
  char *end; // pre funkciu strtod
  int err = 0; // chybovy kod
  double vyskaPr = 1.5, alfa = 0.0, beta = 0.0, vyskaObj = 0.0, vzd = 0.0;
  // premenne vyskaPristroja, alfa, beta, vyskaObjektu a vzdialenost, typ double
  
  if (strcmp(argv[currArg],"-c") == 0)
  {
    if (argc-1 < currArg+1)
    {
      fprintf(stderr,errmsg[ENEARGS]); // za -c nie je dalsi argument
      return EXIT_FAILURE;
    }
    vyskaPr = strtod(argv[currArg+1],&end);
    if (*end || isnan(vyskaPr))
      err = err? err : EARGNAN; // argument nie je cislo
    if (vyskaPr <= 0.0 || vyskaPr > 100.0)
      err = err? err : EILLMHT; //vyska pristroja nie je v intervale (0.0;100.0>   
    currArg += 2;
    if (err)
    {
      fprintf(stderr, errmsg[err]);
      return EXIT_FAILURE;
    }
  }
  
  if (currArg == argc)
  {
    fprintf(stderr,errmsg[EANGNST]); // vyska bola dana, ale uhol nie
    return EXIT_FAILURE;
  }
  
  if (strcmp(argv[currArg],"-m") == 0)
  {
    if (argc-1 < currArg+1)
    {
      fprintf(stderr,errmsg[ENEARGS]); // za -m nie je dalsi argument
      return EXIT_FAILURE;
    }
    alfa = strtod(argv[currArg+1],&end);
    if (*end || isnan(alfa))
      err = err? err : EARGNAN; // argument nie je cislo
    if (alfa <= 0.0 || alfa > 1.4)
      err = err? err : EILLANG; // uhol alfa nie je v spravnom rozsahu
    if ((currArg+2)<=(argc-1))
    {
      beta = strtod(argv[currArg+2],&end);
      if (*end || isnan(beta))
        err = err? err : EARGNAN; // argument nie je cislo
      if (beta <= 0.0 || beta > 1.4)
        err = err? err : EILLANG; // uhol beta nie je v spravnom rozsahu
    }
         
    if (err)
    {
      fprintf(stderr,errmsg[err]);
      return EXIT_FAILURE;
    }
    
    vzd = vyskaPr / cfrac_tan(alfa,getIterCount(alfa));      
    if (beta != 0.0)
      vyskaObj = vyskaPr+cfrac_tan(beta,getIterCount(beta))*vzd;
      
    printf("%.10e\n",vzd); // tlacime vzdialenost na standardny vystup
    if (beta != 0.0)
      printf("%.10e\n",vyskaObj); // tlacime vysku na standardny vystup, ak beta  
                                  // bola dana
    return EXIT_SUCCESS;   
  }
  else
  {
    fprintf(stderr,errmsg[EANGNST]); // vyska bola dana, ale uhol nie
    return EXIT_FAILURE;
  }
}


// Jednoducha funkcia, ktora len rozhodne, ktoru dalsiu funkciu ma na zaklade
// prveho argumentu zavolat
int spracujArgumenty(int argc, char *argv[])
{
  if (argc == 1) // Bez argumentov
  {
    fprintf(stderr,errmsg[ENOARGS]);
    return EXIT_FAILURE;
  }
  else if (strcmp(argv[1],"--help") == 0) // Vypise help
  {
    printf(help);
    return EXIT_SUCCESS;
  }
  else if (strcmp(argv[1],"--tan") == 0) // Porovnavanie presnosti vypoctu tangens
  {
    if (spracujTan(argc,argv))
      return EXIT_FAILURE;
  }
  else if (strcmp(argv[1],"-m") == 0 || strcmp(argv[1],"-c") == 0) // Meranie vzdialenosti a vysky
  {
    if (spracujMeranieVysky(argc,argv))
      return EXIT_FAILURE;
  }
  else
  {
    fprintf(stderr,errmsg[EILLARGS]); // nespravne argumenty
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


// Vyuzita len pre volanie funkcie spracujArgumenty, ktora nasledne vykona
// pozadovanu cinnost programu
int main(int argc, char *argv[])
{  
  // Ak sa vyuziva funkcia tato funkcia tak je potrebne vykomentovat volanie
  // funkcie spracujArgumenty, pretoze vyhodi chybu pri pozadovanych argumentoch
  // pre funkciu iterationCounter
//iterationCounter(atoi(argv[1]));
  
  if(spracujArgumenty(argc, argv))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
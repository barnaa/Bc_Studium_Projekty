// Subor: h2o.c
// Autor: Andrej Barna (xbarna01)
// Predmet: IOS - Projekt 2
// Tema: Synchronizacia procesov - Tvorba H2O
// Ak. rok: 2014/2015


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>


// Struktura Shared Memory Segment
struct smems{

  unsigned hydnum;      // Pocet vodikov cakajucich na tvorbu molekuly
  unsigned oxynum;      // Pocet kyslikov cakajucich na tvorbu molekuly
  unsigned printnum;    // 'A' = ID vypisu
  unsigned deathnum;    // Pocet atomov, ktore cakaju na zanik
  unsigned bsynwq;      // Bonding Synchronization Waiting Queue - 0/1/2

  sem_t *bsyn;          // bonding synchronization
  sem_t *bsynws;        // bonding synchronization write semaphor
  sem_t *s_write;       // semaphor write
  sem_t *oQueue;        // oxygen queue
  sem_t *hQueue;        // hydrogen queue
  sem_t *aaw;           // atom at work
  sem_t *deathQueueH;   // semafor pre smrt vodikov
  sem_t *deathQueueO;   // semafor pre smrt kyslikov
  sem_t *deathCountL;   // semafor pre pocitadlo spojenych atomov

  FILE *outFile;        // Kam sa zapise vystup = h2o.out
};


// Mozne stavy atomu
enum atomStateID {START, WAIT, READY, BBOND, EBOND, FIN, };

// Textove formy statov pre vypis
const char * statemsg[] = {
  [START] = "started",
  [WAIT]  = "waiting",
  [READY] = "ready",
  [BBOND] = "begin bonding",
  [EBOND] = "bonded",
  [FIN]   = "finished",
};


// ID Chybovych stavov
enum errNo {EOK, ENEN, EWA1, EWA2, EWA3, EWA4, EFO, EOGEN, EHGEN, EGENO, EGENH,
            ESYS, EMAP, ESEM, };

// Textove formy statov pre vypis
const char * errmsg[] = {
  [EOK]   = "Life is good.\n",
  [ENEN]  = "Nedostatocny pocet argumentov!\n",
  [EWA1]  = "Pocet molekul musi byt cele kladne cislo!\n",
  [EWA2]  = "Maximalna doba generovania vodika musi byt cele kladne cislo\
 mensie nez 5001!\n",
  [EWA3]  = "Maximalna doba generovania kyslika musi byt cele kladne cislo\
 mensie nez 5001!\n",
  [EWA4]  = "Maximalna doba tvorby molekuly musi byt cele kladne cislo mensie\
 nez 5001!\n",
  [EFO]   = "Chyba pri otvarani suboru h2o.out!\n",
  [EOGEN] = "Chyba pri tvoreni kysliku!\n",
  [EHGEN] = "Chyba pri tvoreni vodiku!\n",
  [EGENO] = "Chyba pri tvoreni generatora kysliku!\n",
  [EGENH] = "Chyba pri tvoreni generatora vodiku!\n",
  [ESYS] = "Chyba systemoveho volania!\n",
  [EMAP] = "Chyba pri mapovani pamate!\n",
  [ESEM] = "Chyba pri praci so semaforom!\n",
};


void bond(const char * atom, unsigned atomID, int bTime, struct smems *smem);
void printRecord(const char * atom, unsigned atomID, unsigned atomState,
                 struct smems *smem);
void syncAfterBonding(const char *atom, unsigned number, struct smems *smem);
void incFinAtomCounter(unsigned ocount, struct smems *smem);
void bondReqCheck(const char * atom, unsigned number, struct smems *smem);
void ssem_wait(sem_t *sem);
void ssem_post(sem_t *sem);
void ssem_destroy(struct smems *smem);



int main(int argc, char **argv){

  /////////////////////////////////////////////////////////////////////////////

  // Spracovanie argumentov

  if(argc < 5){
    fprintf(stderr,errmsg[ENEN]);
    return EXIT_FAILURE;
  }

  char *str;
  int ocount, hgent, ogent, btime;

  ocount = strtol(argv[1],&str,10);
  if(*str || ocount < 1){
    fprintf(stderr,errmsg[EWA1]);
    return EXIT_FAILURE;
  }

  hgent = strtol(argv[2],&str,10);
  if(*str || hgent < 0 || hgent > 5000){
    fprintf(stderr,errmsg[EWA2]);
    return EXIT_FAILURE;
  }

  ogent = strtol(argv[3],&str,10);
  if(*str || ogent < 0 || ogent > 5000){
    fprintf(stderr,errmsg[EWA3]);
    return EXIT_FAILURE;
  }

  btime = strtol(argv[4],&str,10);
  if(*str || btime < 0 || btime > 5000){
    fprintf(stderr,errmsg[EWA4]);
    return EXIT_FAILURE;
  }

  /////////////////////////////////////////////////////////////////////////////


  // Mapovanie struktury do zdielanej pamate
  struct smems *smem;
  if((smem = mmap(NULL, sizeof(struct smems), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED){
    fprintf(stderr,errmsg[EMAP]);
    exit(2);
  }

  // Inicializacia premennych
  smem->oxynum = 0;
  smem->hydnum = 0;
  smem->bsynwq = 0;
  smem->deathnum = 0;
  smem->printnum = 1;

  unsigned error = EOK;

  // Mapovanie semaforov a ich inicializacia
  if((smem->bsyn = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->bsynws = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->s_write = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->aaw = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->oQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, 
                          MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->hQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->deathQueueH = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->deathQueueO = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;
  if((smem->deathCountL = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, 3, 0)) == MAP_FAILED)
    error = EMAP;

  if(error){
    fprintf(stderr,errmsg[error]);
    exit(2);
  }

  if(sem_init(smem->bsyn, 1, 0))
    error = ESEM;
  if(sem_init(smem->bsynws, 1, 1))
    error = ESEM;
  if(sem_init(smem->s_write, 1, 1))
    error = ESEM;
  if(sem_init(smem->aaw, 1, 1))
    error = ESEM;
  if(sem_init(smem->oQueue, 1, 0))
    error = ESEM;
  if(sem_init(smem->hQueue, 1, 0))
    error = ESEM;
  if(sem_init(smem->deathQueueH, 1, 0))
    error = ESEM;
  if(sem_init(smem->deathQueueO, 1, 0))
    error = ESEM;
  if(sem_init(smem->deathCountL, 1, 1))
    error = ESEM;

  // Otvaranie vystupneho suboru
  smem->outFile = fopen("h2o.out","w");
  if(smem->outFile == NULL)
    error = EFO;

  if(error){
    fprintf(stderr,errmsg[error]);
    ssem_destroy(smem);
    exit(2);
  }

///////////////////////////////////////////////////////////////////////////////

  pid_t ogenpid, hgenpid; // OxygenGeneratorPID, HydrogenGeneratorPID

  // Tvorba generatora kyslikov
  ogenpid = fork();

  if(!ogenpid){
  /////////////////////////////////////////////////////////////////////////////
    // OXYGEN GENERATOR
    ///////////////////////////////////////////////////////////////////////////

    // Zmeni si skupinu, aby sa potom dal zabit spolu s atomami v hlavnom
    // procese a teda vratit spravny navratovy kod
    setpgid(0,0);
    srand(getpid());
    pid_t opid;

    for(int i = 0; i < ocount; i++){
      // Uspi sa na dobu v intervale na generovanie kyslikov
      usleep(rand()%(ogent+1));

      // Vytvori child process
      opid = fork();
      if(!opid){
      ///////////////////////////////////////
        // OXYGEN
        ///////////////////////////////////////

        // Vypise, ze vznikol a zisti, ci pracuje nejaky atom, resp. ci sa
        // tvori molekula a da sa do semaforu
        printRecord("O", i+1, START, smem);
        ssem_wait(smem->aaw);

        // Moze pokracovat - inkrementuje pocitadlo a skontroluje, ze ci je
        // dostatocny pocet atomov
        smem->oxynum++;
        bondReqCheck("O", i, smem);
      
        // Atom sa tu bud pri Waiting zastavi a uvolni sa pri dostatku atomov,
        // alebo len tadialto prejde
        ssem_wait(smem->oQueue);

        // Tvorenie molekuly
        bond("O", i+1, btime, smem);
      
        // Atomy sa pockaju a vytlacia spravu o ukonceni spajania
        syncAfterBonding("O", i, smem);
      
        // Atom sa pripocita k spojenym atomom a ak je posledny, tak uvolni
        // semafory na ukoncenie procesov
        incFinAtomCounter(ocount, smem);

        // Atom caka na ukoncenie cinnosti ostatnych atomov
        ssem_wait(smem->deathQueueO);

        // Atom ohlasi svoje ukoncenie a zanikne
        printRecord("O", i+1, FIN, smem);
        exit(EXIT_SUCCESS);
      /////////////////////////////////////
      }
      else if(opid < 0){
        fprintf(stderr,errmsg[EOGEN]);
        // Uprace po sebe procesy, aj ked bol zabity hlavny proces, avsak
        // nasledne vrati navratovy kod funkcie kill()
        if(kill(getppid(),0))
          killpg(0, 9);
        exit(2);
      }
    }

    // Generator tu caka na smrt svojich potomkov - kyslikov
    int status;
    for(int d = 0; d < ocount; d++){
      wait(&status);
      if(WIFEXITED(status))
        if(WEXITSTATUS(status)){
          if(kill(getppid(),0))
            killpg(0, 9);
          exit(2);
        }
      ssem_post(smem->deathQueueO);
    }

    exit(EXIT_SUCCESS);
  /////////////////////////////////////////////////////////////////////////////
  }
  else if (ogenpid > 0){
    // Kod vykonany rodicom - tvorba generatora vodikov
    hgenpid = fork();
    if(!hgenpid){
    ///////////////////////////////////////////////////////////////////////////
      // HYDROGEN GENERATOR
      /////////////////////////////////////////////////////////////////////////

      // Zmeni si skupinu, aby sa potom dal zabit spolu s atomami v hlavnom
      // procese a teda vratit spravny navratovy kod
      setpgid(0,0);
      srand(getpid());
      pid_t hpid;

      for(int i = 0; i < ocount*2; i++){
        // Uspi sa na dobu v intervale na generovanie vodikov
        usleep(rand()%(hgent+1));

        // Vytvori child process
        hpid = fork();
        if(!hpid){
        ///////////////////////////////////////
          // HYDROGEN
          ///////////////////////////////////////

          // Vypise, ze vznikol a zisti, ci pracuje nejaky atom, resp. ci sa
          // tvori molekula a da sa do semaforu
          printRecord("H", i+1, START, smem);
          ssem_wait(smem->aaw);

          // Moze pokracovat - inkrementuje pocitadlo a skontroluje, ze ci je
          // dostatocny pocet atomov
          smem->hydnum++;
          bondReqCheck("H", i, smem);

          // Atom sa tu bud pri Waiting zastavi a uvolni sa pri dostatku
          // atomov, alebo len tadialto prejde
          ssem_wait(smem->hQueue);

          // Tvorenie molekuly
          bond("H", i+1, btime, smem);
          
          // Atomy sa pockaju a vytlacia spravu o ukonceni spajania
          syncAfterBonding("H", i, smem);

          // Atom sa pripocita k spojenym atomom a ak je posledny, tak uvolni
          // semafory na ukoncenie procesov
          incFinAtomCounter(ocount, smem);

          // Atom caka na ukoncenie cinnosti ostatnych atomov
          ssem_wait(smem->deathQueueH);

          // Atom ohlasi svoje ukoncenie a zanikne
          printRecord("H", i+1, FIN, smem);
          exit(EXIT_SUCCESS);
        ///////////////////////////////////////
        }
        else if(hpid < 0){
          // Ak nastane chyba, tak ukonci vsetky svoje vzniknute procesy
          // a ukonci aj seba
          fprintf(stderr,errmsg[EHGEN]);
          // Uprace po sebe procesy, aj ked bol zabity hlavny proces, avsak
          // nasledne vrati navratovy kod funkcie kill()
          if(kill(getppid(),0))
            killpg(0, 9);
          exit(2);
        }
      }

      // Generator tu caka na smrt svojich potomkov - vodikov
      int status;
      for(int d = 0; d < ocount*2-1; d++){
        wait(&status);
        if(WIFEXITED(status))
          if(WEXITSTATUS(status)){
            if(kill(getppid(),0))
              killpg(0, 9);
            exit(2);
          }
        ssem_post(smem->deathQueueH);
      }

      exit(EXIT_SUCCESS);
    ///////////////////////////////////////////////////////////////////////////
    }
    else if(hgenpid > 0){
      // Hlavny proces caka na smrt generatora vodikov
      int status;
      waitpid(-1, &status, 0);
      if(WIFEXITED(status))
        if(WEXITSTATUS(status)){
          killpg(ogenpid, 9);
          killpg(hgenpid, 9);
          ssem_destroy(smem);
          exit(2);
        }
    }
    else{
      // Chyba pri tvoreni potomka - generatora vodika
      fprintf(stderr,errmsg[EGENH]);
      ssem_destroy(smem);
      exit(2);
    }

    // Hlavny proces caka na smrt generatora kyslikov
    int status;
    waitpid(-1, &status, 0);
    if(WIFEXITED(status))
      if(WEXITSTATUS(status)){
        killpg(hgenpid, 9);
        killpg(ogenpid, 9);
        ssem_destroy(smem);
        exit(2);
      }
  }
  else{
    // Chyba pri tvoreni potomka - generatora kyslika
    fprintf(stderr,errmsg[EGENO]);
    ssem_destroy(smem);
    exit(2);
  }
  
  // Znicia sa semafory
  ssem_destroy(smem);

  return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////


// Zapise zaznam, ze sa atom ide spajat a nasledne uspi proces na ziadany cas
void bond(const char * atom, unsigned atomID, int bTime, struct smems *smem){
  printRecord(atom, atomID, BBOND, smem);
  srand(getpid());
  usleep(rand()%(bTime+1));
  return;
}


// Vypise do suboru pozadovane informacie a inkrementuje pocitadlo
void printRecord(const char * atom, unsigned atomID, unsigned atomState,
                                                           struct smems *smem){
  ssem_wait(smem->s_write);
  fprintf(smem->outFile,"%u\t: %s %u\t: %s\n",smem->printnum, atom, atomID,
                                                          statemsg[atomState]);
  fflush(smem->outFile);
  smem->printnum++;
  ssem_post(smem->s_write);
  return;
}


// Sluzi k synchronizovaniu atomov po tom, ako ukoncia funkciu bond
void syncAfterBonding(const char *atom, unsigned number, struct smems *smem){
  ssem_wait(smem->bsynws);
  
  // Prvy a druhy atom z funkcie bond - cakaju na treti atom
  if(smem->bsynwq != 2){
    smem->bsynwq++;
    ssem_post(smem->bsynws);
    // Teraz atom bude cakat na signal od tretieho atomu, aby mohol ohlasit,
    // ze dokoncil tvorbu molekuly
    ssem_wait(smem->bsyn);
    printRecord(atom, number+1, EBOND, smem);
    // Ak uz vsetky tri atomy ohlasili, ze ukoncili tvorenie molekuly
    // (pocitadlo je na nule), tak uvolni semafor pre pracu s atomom
    ssem_wait(smem->bsynws);
    if(!smem->bsynwq)
      ssem_post(smem->aaw);
    else
      smem->bsynwq--;
    ssem_post(smem->bsynws);
  }
  // Posledny atom prichadzajuci z funkcie bond - ohlasi ukoncenie tvorby
  // molekuly a uvolni ostatne 2 prvky
  else{
    printRecord(atom, number+1, EBOND, smem);
    ssem_post(smem->bsyn);
    ssem_post(smem->bsyn);
    smem->bsynwq--;
    ssem_post(smem->bsynws);
  }

  return;
}


// Increment Finished Atom Counter - pocita atomy, ktore skoncili svoju cinnost
// a ak uz vsetky skoncili, tak uvolni semafory na ich ukoncenie
void incFinAtomCounter(unsigned ocount, struct smems *smem){
  ssem_wait(smem->deathCountL);
  smem->deathnum++;

  if(smem->deathnum == ocount*3){
    ssem_post(smem->deathQueueO);
    ssem_post(smem->deathQueueH);
  }

  ssem_post(smem->deathCountL);
  return;
}


// Skontroluje, ci existuje dostatok atomov pre tvorbu molekuly - ak ano, tak
// pokracuje dalej, ak nie, tak caka
void bondReqCheck(const char * atom, unsigned number, struct smems *smem){

  if(smem->oxynum && smem->hydnum > 1){
    smem->hydnum -= 2;
    smem->oxynum--;
    printRecord(atom, number+1, READY, smem);
    ssem_post(smem->oQueue);
    ssem_post(smem->hQueue);
    ssem_post(smem->hQueue);
  }
  else{
    printRecord(atom, number+1, WAIT, smem);
    ssem_post(smem->aaw);
  }

  return;
}


// Secure Semaphor Wait - pri chybe vypise chybove hlasenie a ukonci proces
void ssem_wait(sem_t *sem){
  if(sem_wait(sem)){
    fprintf(stderr,errmsg[ESEM]);
    exit(2);
  }
}


// Secure Semaphor Post - pri chybe vypise chybove hlasenie a ukonci proces
void ssem_post(sem_t *sem){
  if(sem_post(sem)){
    fprintf(stderr,errmsg[ESEM]);
    exit(2);
  }
}


// Znici vsetky semafory
void ssem_destroy(struct smems *smem){
  unsigned error = EOK;

  if(sem_destroy(smem->bsyn))
    error = ESEM;
  if(sem_destroy(smem->bsynws))
    error = ESEM;
  if(sem_destroy(smem->s_write))
    error = ESEM;
  if(sem_destroy(smem->aaw))
    error = ESEM;
  if(sem_destroy(smem->oQueue))
    error = ESEM;
  if(sem_destroy(smem->hQueue))
    error = ESEM;
  if(sem_destroy(smem->deathQueueH))
    error = ESEM;
  if(sem_destroy(smem->deathQueueO))
    error = ESEM;
  if(sem_destroy(smem->deathCountL))
    error = ESEM;

  // Namapovany priestor sa automaticky odmapuje pri ukonceni programu, teda
  // nie je potrebne ho odmapovat 

  if(error){
    fprintf(stderr,errmsg[error]);
    exit(2);
  }

  return;
}
  // tail.c
  // Riesenie IJC-DU2, priklad a), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Prepis POSIXovej funkcie tail do jazyka C

  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef LINMAXLEN
  #define LINMAXLEN 512
#endif


int main(int argc, char **argv){
  
  // Default nastavenia
  FILE *srcFile = stdin; // Zdroj
  int fromLine = 0; // Bool - ci sa pocita od riadku alebo n poslednych riadkov
  int lineCount = 10; // Default pocet riadkov
  
  // Spracovanie argumentov
  int cArg = 1;

  if(argc-1){
    // Specifikovany pocet riadkov, resp. od ktoreho riadku sa ma vypisovat
    if(!strcmp(argv[cArg],"-n")){
      if(argc<3){
        fprintf(stderr,"Chyba ciselny argument!\n");
        return 1;
      }
     
      // Od ktoreho riadku pocitat - cislo s plusom
      char *strPtr;
      if(*argv[cArg+1]=='+'){
        lineCount = strtol(argv[cArg+1]+1,&strPtr,10);
        if(*strPtr || lineCount < 1){
          fprintf(stderr, "Pocet riadkov v nespravnom formate!\n");
          return 1;
        }
        fromLine = 1;
      }
 
      // Kolko riadkov pocitat
      else{
        lineCount = strtol(argv[cArg+1],&strPtr,10);
        if(*strPtr || lineCount < 1){
          fprintf(stderr, "Pocet riadkov v nespravnom formate!\n");
          return 1;
        }
      }
      cArg += 2;
    }

    // Zadany nazov suboru
    if(argc-cArg){
      if((srcFile = fopen(argv[cArg],"r")) == NULL){
        fprintf(stderr, "Chyba pri otvarani suboru!\n");
        return 1;
      }
    }

  }
  

  char str[LINMAXLEN]; // Buffer string
  int ln = 1; // Line Number
  int lof = 0; // Line OverFlow

  // Vypis od zadaneho riadku
  if(fromLine){
    while(fgets(str,LINMAXLEN,srcFile)){
      if(strlen(str) == LINMAXLEN-1 && str[LINMAXLEN-2] != '\n'){
        if(!lof){
          fprintf(stderr,"WARNING: Riadok %d ma vacsiu dlzku nez je povolena velkost!\n",ln);
          lof = 1;
        }
        str[LINMAXLEN-2] = '\n';
        int c;
        do ; while((c = fgetc(srcFile)) != '\n');
      }

      // Vypis riadkov
      if(ln >= lineCount)
        printf("%s",str);
      ln++;
    }
  }

  // Vypis n poslednych riadkov
  else{
    // Alokacia priestoru pre riadky
    char *tailt = malloc(LINMAXLEN*lineCount*sizeof(char));
    if(tailt == NULL){
      fprintf(stderr,"ERROR: Nedostatok pamati!\n");
      if(srcFile != stdin)
        fclose(srcFile);
      return EXIT_FAILURE;
    }

    // Prechod suborom a ukladanie riadkov
    while(fgets(str,LINMAXLEN,srcFile)){
      if(strlen(str) == LINMAXLEN-1 && str[LINMAXLEN-2] != '\n'){
        if(!lof){
          fprintf(stderr,"WARNING: Riadok %d ma vacsiu dlzku nez je povolena velkost!\n",ln);
          lof = 1;
        }
        str[LINMAXLEN-2] = '\n';
        int c;
        do ; while((c = fgetc(srcFile)) != '\n');
      }
      strcpy(&tailt[LINMAXLEN*((ln-1)%lineCount)],str);
      ln++;
    }

    // Vypis
    if(ln <= lineCount)
      for(int i = 0; i < ln-1; i++)
        printf("%s", &tailt[i*LINMAXLEN]);
    else{
      int iter = 0;
      for(int i = 0; i < lineCount; i++){
	printf("%s", &tailt[LINMAXLEN*((ln-1)%lineCount+iter)]);
        iter++;
        if(iter+(ln-1)%lineCount == lineCount)
          iter-=lineCount;
      }
    }

    // Uvolnenie alokovaneho miesta
    free(tailt);
    tailt = NULL;
  }
  
  // Zavretie suboru
  if(srcFile != stdin)
    fclose(srcFile);

  return EXIT_SUCCESS;
}

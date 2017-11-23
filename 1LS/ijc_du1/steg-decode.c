  // steg-decode.c
  // Riesenie IJC-DU1, priklad b), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Odhali a vypise tajnu spravu zakodovanu v obrazku danom prvym
  //        argumentom
  
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ppm.h"
#include "error.h"
#include "bit-array.h"
#include "eratosthenes.h"

#ifndef MAXRES
  #define MAXRES 5000
#endif

/*
 * Nacita obrazok dany argumentom programu do struktury ppm, nasledne si vytvori
 * bitove pole odpovedajuce maximalnej velkosti obrazka a pomocou Eratosthenovho
 * sita vyfiltruje v tom poli prvocisla. Nasledne z obrazka ziska spravu, ktora
 * je zasifrovana po bitoch na prvociselnych indexoch v bitovom poli.
 */
int main(int argc, char **argv){
  if(argc<2)
    FatalError("Program spusteny bez argumentov!\n");
  struct ppm *p = ppm_read(argv[1]);
  if(!p)
    FatalError("Chyba pri citani obrazka!\n");
  BA_create(bp,(MAXRES*MAXRES*3));
  Eratosthenes(bp);
  BA_set_bit(bp,0,1);
  short r, c;
  r = 0;
  c = 0;
  unsigned long i;
  for(i = 0; i<p->xsize*p->ysize*3; i++){
    if(!BA_get_bit(bp,i)){
      c+=(1<<r)*(p->data[i+1]&1);
      r++;
      if(r == 8){
        r = 0;
        if(c){
          if(!isprint(c)){
            free(p);
            FatalError("V sprave sa nachadza netlacitelny znak!\n");
          }
          putchar(c);
          c = 0;
        }
        else
          break;
      }
    }
  }
  if(c){
    free(p);
    FatalError("Sprava nie je ukoncena nulovym znakom!\n");
  }
  printf("\n");
  free(p);
  return 0;
}

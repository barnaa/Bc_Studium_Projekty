  // io.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Obsahuje funkciu fgetw, ktora nacita zo suboru f dalsie slovo, tj.
  //        postupnost znakov oddelenu bielymi znakmi. Vracia dlzku slova, inak
  //        vracia EOF ak sa dostane na koniec suboru.

  
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "io.h"


int fgetw(char *s, int max, FILE *f){
 
  if(s == NULL || max < 1 || f == NULL){
    fprintf(stderr,"ERROR: Funkcia spustena s neplatnymi parametrami!\n");
    return 0;
  }
  
  // Znak a pozicia v stringu
  int c, pos = 0;
  
  // Odstranenie bielych znakov pred slovom
  while(isspace(c = fgetc(f)));
  if(c == EOF)
    return EOF;

  // Posledny nacitany znak uz nie je biely - zaciatok slova
  s[pos]=(char) c;
  pos++;

  // Nacitanie zvysku slova
  for( ; pos < max; pos++){
    if(isspace(c = fgetc(f)) || c == EOF){
      s[pos]='\0';
      return (c == EOF) ? EOF : pos;
    }
    s[pos]=(char) c;
  }

  // Bola prekrocena max. dlzka - inak by sa program ukoncil v tele cyklu for
  // Teda vkladame koncovu nulu
  s[pos] = '\0';

  // Splachnutie zvysku pridlheho slova
  while(!isspace(c = fgetc(f)));

  return pos;
}

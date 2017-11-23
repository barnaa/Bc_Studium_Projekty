  // htab_free.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Uvolni tabulku t

  
#include <stdlib.h>
#include <stdio.h>
#include "htable.h"


void htab_free(struct htab_t *t){

  if(t == NULL){
    fprintf(stderr,"ERROR: Tabulka neexistuje!\n");
    return;
  }

  // Volanie clear = zmazanie poloziek a uvolnenie samotnej tabulky 
  htab_clear(t);
  free(t);
  t = NULL;

  return;
}

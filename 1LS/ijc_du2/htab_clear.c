  // htab_clear.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Vymaze vsetky polozky v tabulke t

  
#include <stdlib.h>
#include <stdio.h>
#include "htable.h"


void htab_clear(struct htab_t *t){

  if(t == NULL){
    fprintf(stderr,"ERROR: Ukazatel na tabulku je neplatny!\n");
    return;
  }

  // Prechod vsetkymi polozkami tabulky a ich mazanie
  struct htab_listitem *nitem = NULL;
  for(unsigned i = 0; i < t->htab_size; i++){
    while(t->item[i] != NULL){
      nitem = t->item[i]->next;
      free((t->item[i])->key);
      free(t->item[i]);
      t->item[i] = nitem;
    }
  }

  return;
}

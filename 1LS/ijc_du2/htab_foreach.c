  // htab_foreach.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Funkcia htab_foreach, ktora vykona funkciu danu druhym parametrom na
  //        vsetkych polozkach v tabulke t. Predpis funkcie je
  //          void function(key, data)
 
 
#include <stdlib.h>
#include <stdio.h>
#include "htable.h"


void htab_foreach(struct htab_t *t, void (*function)(const char *, unsigned)){

  if(t == NULL || (*function) == NULL){
    fprintf(stderr,"ERROR: Tabulka alebo funkcia neexistuje!\n");
    return;
  }
 
  // Prechod vsetkymi polozkami tabulky a aplikacia funkcie na ne
  struct htab_listitem *item = NULL;
  for(int i = 0; i < t->htab_size; i++){
    item = t->item[i];
    while(item != NULL){
      function(item->key,item->data);
      item = item->next;
    }
  }

  return;
}

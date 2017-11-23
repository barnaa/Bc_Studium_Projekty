  // htab_init.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Vytvori a inicializuje tabulku o pozadovanej velkosti
 
 
#include <stdlib.h>
#include "htable.h"


struct htab_t *htab_init(unsigned size){

  if(!size)
    return NULL;

  // Alokovanie pamate pre tabulku
  struct htab_t *table = NULL;
  table = malloc(sizeof(struct htab_t)+size*sizeof(struct htab_listitem *));
  if(table == NULL)
    return NULL;

  // Ulozenie velkosti do tabulky a inicializacia
  table->htab_size = size;
  for(unsigned i = 0; i < size; i++)
    table->item[i] = NULL;

  return table;
}

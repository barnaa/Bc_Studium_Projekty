  // htab_lookup.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Vyhlada v tabulke zaznam a ak ho nenajde, tak ho automaticky vytvori

  
#include <stdlib.h>
#include <string.h>
#include "htable.h"


struct htab_listitem *htab_lookup(struct htab_t *t, const char *key){

  if(t == NULL || key == NULL)
    return NULL;

  // Kopia vstupneho retazca - ak by predosly zanikol pocas hladania
  char *ckey = malloc((strlen(key)+1)*sizeof(char));
  if(ckey == NULL)
    return NULL;
  strcpy(ckey, key);

  // Ukazatel na predoslu a aktualnu polozku
  struct htab_listitem *pitem = NULL;
  struct htab_listitem *item = t->item[hash_function(ckey, t->htab_size)];
  
  // Prechadzanie prislusnej vetvy v tabulke
  while(item != NULL){
    if(!strcmp(item->key, ckey))
      break;
    pitem = item;
    item = item->next;
  }

  // Tvorenie polozky, ak nebola najdena
  if(item == NULL){
    item = malloc(sizeof(struct htab_listitem));
    if(item == NULL){
      free(ckey);
      return NULL;
    }
    
    item->key = malloc((strlen(ckey)+1)*sizeof(char));
    if(item->key == NULL){
      free(item);
      free(ckey);
      return NULL;
    }

    if(pitem == NULL)
      t->item[hash_function(ckey, t->htab_size)] = item;
    else
      pitem->next = item;
    strcpy(item->key, ckey);
    item->data = 0;
    item->next = NULL;
  }
  
  // Uvolenenie docasnej kopie kluca
  free(ckey);

  return item;
}

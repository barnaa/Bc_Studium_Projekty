  // htab_remove.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Funkcia htab_remove, ktora odstrani z tabulky prvok s pozadovanym 
  //        klucom
  
  
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "htable.h"


void htab_remove(struct htab_t *t, const char *key){

  if(t == NULL || key == NULL){
    fprintf(stderr,"ERROR: Tabulka alebo kluc neexistuje!\n");
    return;
  }

  // Kopia kluca - ak by predosly zanikol pocas vyhladavania
  char *ckey = malloc(strlen(key)+1);
  if(ckey == NULL){
    fprintf(stderr,"ERROR: Nepodarilo sa alokovat pamat pre zalohu kluca!\n");
    return;
  }
  strcpy(ckey, key);
  
  // Hladanie a mazanie polozky (ak existuje)
  struct htab_listitem *pitem = NULL;
  struct htab_listitem *item = t->item[hash_function(key,t->htab_size)];
  while(item != NULL){
    if(!strcmp(item->key, ckey)){
      if(pitem == NULL)
        t->item[hash_function(ckey, t->htab_size)] = item->next;
      else
        pitem->next = item->next;
      free(item->key);
      free(item);
      break;
    }
    pitem = item;
    item = item->next;
  }
  
  // Uvolnenie docasnej kopie kluca
  free(ckey);

  return;
}

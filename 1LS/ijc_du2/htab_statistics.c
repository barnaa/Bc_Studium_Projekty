  // htab_statistics.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Funkcia htab_statistics vypise priemernu, najvacsiu a najmensiu
  //        dlzku zoznamov v tabulke t

  
#include <stdio.h>
#include "htable.h"


void htab_statistics(struct htab_t *t){
  
  if(t == NULL){
    fprintf(stderr,"ERROR: Tabulka neexistuje!\n");
    return;
  }

  // Pole s dlzkami zoznamov
  unsigned ctable[t->htab_size];
  for(int i = 0; i < t->htab_size; i++)
    ctable[i] = 0;

  // Pocitanie dlzky zoznamov
  struct htab_listitem *itemi;
  for(unsigned i = 0; i < t->htab_size; i++){
    itemi = t->item[i];
    while(itemi != NULL){
      ctable[i]++;
      itemi = itemi->next;
    }
  }
  
  // Zistovanie minima, maxima a priemeru
  unsigned min, max, sum;
  min = ctable[0];
  max = ctable[0];
  sum = ctable[0];
  for(unsigned i = 1; i < t->htab_size; i++){
    sum += ctable[i];
    if(ctable[i] > max)
      max = ctable[i];
    if(ctable[i] < min)
      min = ctable[i];
  }

  // Vypis
  printf("Avg: %lf\n",(double)sum/t->htab_size);
  printf("Max: %u\n", max);
  printf("Min: %u\n", min);

  return;
}

  // wordcount.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Pocita pocet vyskytov slov v texte zo standardneho vstupu, pricom
  //        si uklada data do tabulky, z ktorej ich nasledne vypise

  
#include <stdio.h>
#include <stdlib.h>
#include "htable.h"
#include "io.h"
 
                  
#define WORDLEN 127
#define TSIZE 1024
/* Velkost som vyberal ako vhodny kompromis medzi casom pristupu k prvkom
 * tabulky a velkostou rezie pre danu rychlost pristupu - vyssia velkost sice
 * zrychli pristup aj pri vacsom pocte prvkov, ale taktiez vyzaduje viac
 * priestoru. Pri nizsom pocte prvkov tabulky by vsak rezia mohla byt vacsia nez
 * samotny objem dat v tabulke. Pre nizsiu velkost to plati opacne - nizsia
 * rezia, avsak vyssi cas pristupu.
 */


void printItem(const char *key, unsigned data);


int main(){

  // Tvorba tabulky
  struct htab_t *t = htab_init(TSIZE);
  if(t == NULL){
    fprintf(stderr,"CHYBA: Nepodarilo sa alokovat tabulku!\n");
    return EXIT_FAILURE;
  }

  // Alokacia pamate pre retazec nie je potrebna, zalohu si robia funkcie
  //char *str = malloc((WORDLEN+1)*sizeof(char));
  
  // Tvorba ukazatela na aktualny zaznam a retazca
  char str[WORDLEN+1];
  struct htab_listitem *rec = NULL;
  
  // Pocitanie slov
  while(fgetw(str, WORDLEN, stdin) != EOF){
    rec = htab_lookup(t, str);
    if(rec == NULL){
      fprintf(stderr,"Chyba pri alokacii novej polozky %s v tabulke!\n",str);
      //free(str);
      htab_free(t);
      return EXIT_FAILURE;
    }
    rec->data++;
  }

  // Vypis pomocou funkcie printItem
  htab_foreach(t, printItem);

  // Uvolnenie pamate
  //free(str);
  htab_free(t);

  return EXIT_SUCCESS;
}


/**
 * Funkcia printItem tlaci obsah polozky v tabulke na standardny vystup, urcene
 * pre vypis dat z tabulky
 */
void printItem(const char *key, unsigned data){

  printf("%s\t%u\n",key,data);

  return;
}

  // prvocisla.c
  // Riesenie IJC-DU1, priklad a), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Vypise 10 najvacsich prvocisel mensich nez 201000000
  
#include <stdio.h>
#include <stdlib.h>
#include "bit-array.h"
#include "error.h"
#include "eratosthenes.h"

/*
 * Vytvori bitove pole o velkosti 201 milionov bitov, v ktorom nasledne pomocou
 * Eratosthenovho sita vyfiltruje prvocisla a nasledne vypise 10 najvacich
 * prvocisel v poli array.
 */
int main(int argc, char **argv){
  BA_create(array, 201000000);
  Eratosthenes(array);
  unsigned long p;
  int i = 10;
  unsigned long primes[i];
  for(p = array[0];i; p--)
    if(!BA_get_bit(array,p-1)){
      primes[i-1]=p;
      i--;
    }
  for(unsigned long c=0; c<10; c++)
    printf("%lu\n",primes[c]);
  return 0;
}

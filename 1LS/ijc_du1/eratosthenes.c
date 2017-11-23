  // eratosthenes.c
  // Riesenie IJC-DU1, priklad a), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Funkcia Eratosthenes, ktora z bitoveho pola zadaneho argumentom
  //        vyfiltruje prvocisla pomocou tzv. Eratosthenovho sita

#include <math.h> 
#include "bit-array.h"
#include "error.h"
#include "eratosthenes.h"

/*
 * V bitovom poli danom argumentom nastavi bity, ktore nie su na prvociselnom
 * indexe na hodnotu 1, teda na prvociselnych indexoch bude hodnota bitu nula.
 * Indexy su zmensene o 1, aby sa efektivnejsie vyuzivalo bitove pole.
 */
void Eratosthenes(BitArray_t pole){
  unsigned long i, m;
  i = 2;
  while(i <= sqrt(pole[0])){
    for(m = i; m*i <= pole[0]; m++)
      if(!BA_get_bit(pole,m*i-1)) BA_set_bit(pole,m*i-1,1);
    do i++; while(BA_get_bit(pole, i-1));
  }
}


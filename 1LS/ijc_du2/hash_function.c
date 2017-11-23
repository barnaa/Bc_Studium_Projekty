  // bash_function.c
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Hashovacia funkcia pre kniznicu htable.h, prevzate zo zadania DU2


#include "htable.h"


unsigned int hash_function(const char *str, unsigned htab_size){
  unsigned int h = 0;
  const unsigned char *p;
  for(p = (const unsigned char *) str; *p != '\0'; p++)
    h = 65599*h + *p;
  return h % htab_size;
}

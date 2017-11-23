  // io.h
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Hlavickovy subor k io.c, ktory obsahuje prototyp funkcie fgetw
  
#ifndef IOH
  
  #define IOH

  // Nacita prve slovo zo suboru
  int fgetw(char *s, int max, FILE *f);

#endif

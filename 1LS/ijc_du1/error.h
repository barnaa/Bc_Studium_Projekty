  // error.h
  // Riesenie IJC-DU1, priklad b), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Hlavickovy subor k error.c obsahujuci definicie 

#ifndef ERROR_H
  #define ERROR_H
  void Warning(const char *fmt, ...);
  void FatalError(const char *fmt, ...);
#endif

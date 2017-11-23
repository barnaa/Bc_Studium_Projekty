  // ppm.h
  // Riesenie IJC-DU1, priklad b), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Hlavickovy subor k zdrojovemu suboru ppm.c
  
#ifndef PPM_H
  #define PPM_H
  struct ppm{
    unsigned xsize;
    unsigned ysize;
    char data[];
  };
  
  struct ppm * ppm_read(const char * filename);
  int ppm_write(struct ppm *p, const char * filename);
#endif

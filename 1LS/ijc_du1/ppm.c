  // ppm.c
  // Riesenie IJC-DU1, priklad b), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Definicia funkcii pre citanie a zapis obrazkov v P6 ppm formate

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "ppm.h"

/*
 * Nacita do struktury ppm obrazok, ktory je urcenz parametrom funkcie. Vrati
 * NULL ak nastane chyba pri nacitani obrazka.
 */
struct ppm * ppm_read(const char * filename){
  FILE *file = fopen(filename, "rb");
  if (!file){
    Warning("Nepodarilo sa otvorit subor %s!\n",filename);
    return NULL;
  }
  unsigned xs, ys;
  if(fscanf(file, "P6 %u %u 255 ", &xs, &ys)!=2){
    Warning("Chyba pri citani hlavicky obrazka!\n");
    return NULL;
  }
#ifdef MAXRES
  if (xs > MAXRES || ys > MAXRES){
    Warning("Privelky obrazok!\n");
    return NULL;
}
#endif
  struct ppm *p = malloc(sizeof(struct ppm)+xs*ys*sizeof(char)*3);
  if(p==NULL){
    Warning("Nedostatok pamati pre nacitanie obrazka!\n");
    return NULL;
  }
  p->xsize = xs;
  p->ysize = ys;
  int i,c;
  i = 0;
  while((c=fgetc(file))!=EOF){
    if(i>=xs*ys*3*sizeof(char)){
      Warning("V subore %s je ulozenych viac dat nez udava rozmer obrazka!\n",filename);
      free(p);
      return NULL;
    }
    p->data[i]=c;
    i++;
  }
  if(i<xs*ys*sizeof(char)*3){
    Warning("Subor %s neobsahuje kompletne informacie o obrazku!\n",filename);
    free(p);
    return NULL;
  }
  fclose(file);
  return p;
}


/*
 * Zapise obsah struktury danej prvym parametrom do suboru s nazvom danym druhym
 * parametrom ako ppm P6 obrazok. Ak nastane chyba, tak vrati hodnotu -1.
 */
int ppm_write(struct ppm *p, const char * filename){
  FILE *file = fopen(filename, "wb");
  if (!file){
    Warning("Nepodarilo sa otvorit subor %s pre zapis!\n",filename);
    return -1;
  }
  fprintf(file,"P6\n%u %u\n255\n",p->xsize,p->ysize);
  for(unsigned i = 0; i<p->xsize*p->ysize*3;i++)
    fputc(p->data[i],file);
  fclose(file);
  return 0;
}

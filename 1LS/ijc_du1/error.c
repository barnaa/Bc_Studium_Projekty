  // error.c
  // Riesenie IJC-DU1, priklad b), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Definicia funkcii Warning a FatalError pre vypis chybovych hlaseni
  //        na stderr

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"

/*
 * Na chybovy vystup napise hlasenie "CHYBA: <sprava>", pricom sprava je urcena
 * argumentami. Tato funkcia neukoncuje program.
 */
void Warning(const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "CHYBA: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  return;
}

/*
 * Cinnostou podobna ako Warning - vypise spravu urcenu parametrom na chybovy
 * vystup, ale FatalError ukonci program s vystupnym chybovym kodom 1.
 */
void FatalError(const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "CHYBA: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  exit(1);
}

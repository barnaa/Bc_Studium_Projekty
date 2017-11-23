  // htable.h
  // Riesenie IJC-DU2, priklad b), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Hlavickovy subor pre tvorbu kniznice, obsahujuci prototypy funkcii
  //        operujucich nad tabulkou
  

#ifndef HTABLEH

  #define HTABLEH

  // Struktura prvku v tabulke
  struct htab_listitem{
    char *key;
    unsigned data;
    struct htab_listitem *next;
  };

  // Struktura tabulky
  struct htab_t{
    unsigned htab_size;
    struct htab_listitem *item[];
  };

  // Hashovacia funkcia
  unsigned int hash_function(const char *str, unsigned htab_size);

  // Inicializacia tabulky
  struct htab_t *htab_init(unsigned size);
  
  // Vyhladanie polozky v tabulke
  struct htab_listitem *htab_lookup(struct htab_t *t, const char *key);
  
  // Vykonanie funkcie pre kazdu polozku v tabulke
  void htab_foreach(struct htab_t *t, void (*function)(const char *, unsigned));

  // Odstranenie polozky z tabulky s klucom key
  void htab_remove(struct htab_t *t, const char *key);

  // Odstranenie vsetkych poloziek v tabulke
  void htab_clear(struct htab_t *t);

  // Uvolnenie tabulky
  void htab_free(struct htab_t *t);

  // Vypis priemernej, minimalnej a maximalnej dlzky zoznamu v tabulke
  void htab_statistics(struct htab_t *t);

#endif

  // bit-array.h
  // Riesenie IJC-DU1, priklad a), 26.3.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: gcc 4.8.4 na serveri Merlin
  // Popis: Definuje datovy typ BitArray_t, pomocne makra pre manipulaciu
  //        s bitovymi polami, makro pre tvorbu bitoveho pola a makra/inline
  //        funkcie pre pracu s bitovymi polami v zavislosti od toho, ci bola
  //        definovana konstanta USE_INLINE

#ifndef BIT_ARRAY_H
  #define BIT_ARRAY_H

  #include "error.h"

  // Typ BitArray_t, co je vlastne pole unsigned longov, pricom prvy prvok udava velkost pola
  typedef unsigned long * BitArray_t;

  // Ziska hodnotu daneho bitu
  #define DU1_GET_BIT_(p,i) (((p)>>(i))&1)

  // Nastavi bit na zvolenu hodnotu
  #define DU1_SET_BIT_(p,i,b) (p=((b)>(DU1_GET_BIT_(p,i)))?(p)+(1ul<<(i)):(p)+((b)^DU1_GET_BIT_(p,i))*(1ul<<(i)))

  // Vytvori bitove pole o pozadovanej velkosti
  #define BA_create(name, size) \
    unsigned long name[(size-1)/(8*sizeof(unsigned long))+2] = {size, };\
    for(int i=1;i<=(size-1)/(8*sizeof(unsigned long))+1;i++) name[i]=0ul


  #ifndef USE_INLINE // Definicie makier

    // Vrati velkost bitoveho pola
    #define BA_size(name) (name[0])

    // Nastavi bit pola urceny indexom na pozadovanu hodnotu
    #define BA_set_bit(name,index,value) do{\
        if (name[0]<=(index) || (index)<0) FatalError("Index %ld mimo rozsah 0..%ld",(long)index,(long)name[0]);\
        DU1_SET_BIT_((name[(index)/(8*sizeof(unsigned long))+1]), (index)%(8*sizeof(unsigned long)), value?1:0);}while(0)

    // Vrati hodnotu bitu pola urceneho indexom
    #define BA_get_bit(name,index) ((index>=0 && index < name[0])?(DU1_GET_BIT_(name[(index)/(8*sizeof(unsigned long))+1],(index)%(8*sizeof(unsigned long)))):(FatalError("Index %ld mimo rozsah 0..%ld",(long)index,(long)name[0]),1))

  #else // Definicie inline funkcii
    // Vrati velkost bitoveho pola
    static inline unsigned long BA_size(BitArray_t name){
        return name[0];
    }

    // Nastavi bit pola urceny indexom na pozadovanu hodnotu
    static inline void BA_set_bit(BitArray_t name, unsigned long index, unsigned long value){
        if (name[0]<=(index) || (index)<0) FatalError("Index %ld mimo rozsah 0..%ld",(long)index,(long)name[0]);
        DU1_SET_BIT_((name[(index)/(8*sizeof(unsigned long))+1]), (index)%(8*sizeof(unsigned long)), value?1:0);
        return;
    }

    // Vrati hodnotu bitu pola urceneho indexom
    static inline unsigned long BA_get_bit(BitArray_t name, unsigned long index){
        return (index>=0&&index<name[0])?DU1_GET_BIT_(name[(index)/(8*sizeof(unsigned long))+1],(index)%(8*sizeof(unsigned long))):(FatalError("Index %ld mimo rozsah 0..%ld",(long)index,(long)name[0]),1);
    }
  #endif
#endif

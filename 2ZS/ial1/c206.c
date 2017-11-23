	
/* c206.c **********************************************************}
{* Téma: Dvousmìrnì vázaný lineární seznam
**
**                   Návrh a referenèní implementace: Bohuslav Køena, øíjen 2001
**                            Pøepracované do jazyka C: Martin Tuèek, øíjen 2004
**                                            Úpravy: Bohuslav Køena, øíjen 2015
**
** Implementujte abstraktní datový typ dvousmìrnì vázaný lineární seznam.
** U¾iteèným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován promìnnou
** typu tDLList (DL znamená Double-Linked a slou¾í pro odli¹ení
** jmen konstant, typù a funkcí od jmen u jednosmìrnì vázaného lineárního
** seznamu). Definici konstant a typù naleznete v hlavièkovém souboru c206.h.
**
** Va¹ím úkolem je implementovat následující operace, které spolu
** s vý¹e uvedenou datovou èástí abstrakce tvoøí abstraktní datový typ
** obousmìrnì vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu pøed prvním pou¾itím,
**      DLDisposeList ... zru¹ení v¹ech prvkù seznamu,
**      DLInsertFirst ... vlo¾ení prvku na zaèátek seznamu,
**      DLInsertLast .... vlo¾ení prvku na konec seznamu, 
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek, 
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku, 
**      DLDeleteFirst ... zru¹í první prvek seznamu,
**      DLDeleteLast .... zru¹í poslední prvek seznamu, 
**      DLPostDelete .... ru¹í prvek za aktivním prvkem,
**      DLPreDelete ..... ru¹í prvek pøed aktivním prvkem, 
**      DLPostInsert .... vlo¾í nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vlo¾í nový prvek pøed aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... pøepí¹e obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal¹í prvek seznamu,
**      DLPred .......... posune aktivitu na pøedchozí prvek seznamu, 
**      DLActive ........ zji¹»uje aktivitu seznamu.
**
** Pøi implementaci jednotlivých funkcí nevolejte ¾ádnou z funkcí
** implementovaných v rámci tohoto pøíkladu, není-li u funkce
** explicitnì uvedeno nìco jiného.
**
** Nemusíte o¹etøovat situaci, kdy místo legálního ukazatele na seznam 
** pøedá nìkdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodnì komentujte!
**
** Terminologická poznámka: Jazyk C nepou¾ívá pojem procedura.
** Proto zde pou¾íváme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozornìní na to, ¾e do¹lo k chybì.
** Tato funkce bude volána z nìkterých dále implementovaných operací.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální promìnná -- pøíznak o¹etøení chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L pøed jeho prvním pou¾itím (tzn. ¾ádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádìt nad ji¾ inicializovaným
** seznamem, a proto tuto mo¾nost neo¹etøujte. V¾dy pøedpokládejte,
** ¾e neinicializované promìnné mají nedefinovanou hodnotu.
**/
  // Inicializujeme vsetky ukazatele na NULL, aby nemohlo nastat, ze ukazuju
  // niekam do pamate             
  L->First = NULL;
  L->Act = NULL;
  L->Last = NULL;  
  return;
}

void DLDisposeList (tDLList *L) {
/*
** Zru¹í v¹echny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Ru¹ené prvky seznamu budou korektnì
** uvolnìny voláním operace free. 
**/
  // Uvolnime najprv vsetky prvky okrem prveho
	while(L->First != L->Last){
    L->Act = L->First->rptr;
    free(L->First);
    L->First = L->Act;
  } 
  // A potom uvolnime aj posledny prvok a nastavime ukazatele na NULL - aby
  // neukazovali niekde nahodne do pamate
  free(L->First);
  L->First = NULL;
  L->Act = NULL;
  L->Last = NULL;
  return;
}

void DLInsertFirst (tDLList *L, int val) {
/*
** Vlo¾í nový prvek na zaèátek seznamu L.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
  // Ulozime si ukazatel na prvy prvok
  tDLElemPtr tmp = L->First;
  // Vyhradime miesto pre novy prvok
  if((L->First = malloc(sizeof(struct tDLElem))) == NULL)
    DLError();
  else{
    // Pospajame zoznam a spravne nastavime ukazatele
    L->First->rptr = tmp;
    if(tmp != NULL)
      tmp->lptr = L->First;
    else
      L->Last = L->First;
    L->First->lptr = NULL;                 
    L->First->data = val;
  }
  return;
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vlo¾í nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/ 	
  // Ulozime si ukazatel na prvy prvok
  tDLElemPtr tmp = L->Last;
  if((L->Last = malloc(sizeof(struct tDLElem))) == NULL)
    DLError();
  else{    
    // Pospajame zoznam a spravne nastavime ukazatele
    L->Last->lptr = tmp;
    if(tmp != NULL)
      tmp->rptr = L->Last;
    else
      L->First = L->Last;
    L->Last->rptr = NULL;                 
    L->Last->data = val;
  }
  return; 	
}

void DLFirst (tDLList *L) {
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
  L->Act = L->First;
  return;
}

void DLLast (tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
  L->Act = L->Last;
  return;
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
  // Overime ci zoznam nie je prazdny
	if(L->First == NULL)
    DLError();
  else
    // Vratime pozadovane data pomocou dereferencie
    *val = L->First->data;  
  return;
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
  // Overime ci zoznam nie je prazdny
	if(L->Last == NULL)
    DLError();
  else           
    // Vratime pozadovane data pomocou dereferencie
    *val = L->Last->data;
  return;
}

void DLDeleteFirst (tDLList *L) {
/*
** Zru¹í první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
**/
  // Overime ci zoznam nie je prazdny
  if(L->First != NULL){
    // Ak je prvy prvok aktivny tak zoznam deaktivujeme
    if (L->First == L->Act)
      L->Act = NULL;
    // Ulozime si adresu druheho prvku do docasnej premennej, uvolnime pamat a
    // opat prepojime zoznam
	  tDLElemPtr tmp = L->First->rptr;
    free(L->First);
    L->First = tmp;
    // Overime, ci nebol v zozname jediny prvok a prevedieme patricne opatrenia
    if(L->First == NULL)
      L->Last = NULL;
    else             
      L->First->lptr = NULL;
  }
  return;
}	

void DLDeleteLast (tDLList *L) {
/*
** Zru¹í poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
**/  
  // Overime ci zoznam nie je prazdny
  if(L->Last != NULL){
    if (L->Last == L->Act)
      L->Act = NULL;    
    // Ulozime si adresu predposledneho prvku do docasnej premennej, uvolnime 
    // pamat a opat prepojime zoznam
	  tDLElemPtr tmp = L->Last->lptr;
    free(L->Last);
    L->Last = tmp; 
    // Overime, ci nebol v zozname jediny prvok a prevedieme patricne opatrenia
    if(L->Last == NULL)
      L->First = NULL;
    else              
      L->Last->rptr = NULL;
  }
  return; 
}

void DLPostDelete (tDLList *L) {
/*
** Zru¹í prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se nedìje.
**/
  // Kontrola podmienok, ulozenie pointeru na dalsi prvok, uvolnenie pamate
	if(L->Act != NULL && L->Act != L->Last){
    tDLElemPtr tmp = L->Act->rptr->rptr;
    free(L->Act->rptr);
    // Opatovne prepojenie zoznamu a osetrenie pripadu, kedy je odstraneny
    // posledny prvok
    L->Act->rptr = tmp;
    if(tmp != NULL)
      tmp->lptr = L->Act;
    else
      L->Last = L->Act;
  }
	return;
}

void DLPreDelete (tDLList *L) {
/*
** Zru¹í prvek pøed aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se nedìje.
**/
  // Kontrola podmienok, ulozenie pointeru na predosly prvok, uvolnenie pamate
	if(L->Act != NULL && L->Act != L->First){
    tDLElemPtr tmp = L->Act->lptr->lptr;
    free(L->Act->lptr);
    // Opatovne prepojenie zoznamu a osetrenie pripadu, kedy je odstraneny
    // posledny prvok
    L->Act->lptr = tmp;  
    if(tmp != NULL)
      tmp->rptr = L->Act;
    else
      L->First = L->Act;
  }
	return;
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vlo¾í prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
  // Ak zoznam nie je prazdny, tak zazalohuje si ukazatel na dalsi prvok a
  // alokuje pamat
  if(L->Act != NULL){
    tDLElemPtr tmp = L->Act->rptr;
    if((L->Act->rptr = malloc(sizeof(struct tDLElem))) == NULL)
      DLError();
    else{
      // Osetrenie situacie, ked je aktivny prvok posledny, prepojenie zoznamu
      if(L->Act == L->Last)
        L->Last = L->Act->rptr;
      else             
        tmp->lptr = L->Act->rptr;
      L->Act->rptr->rptr = tmp;
      L->Act->rptr->lptr = L->Act;
      L->Act->rptr->data = val;
    }
  }
	return;
}

void DLPreInsert (tDLList *L, int val) {
/*
** Vlo¾í prvek pøed aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
  // Ak zoznam nie je prazdny, tak zazalohuje si ukazatel na dalsi prvok a
  // alokuje pamat
  if(L->Act != NULL){
    tDLElemPtr tmp = L->Act->lptr;
    if((L->Act->lptr = malloc(sizeof(struct tDLElem))) == NULL)
      DLError();
    else{  
      // Osetrenie situacie, ked je aktivny prvok prvy, prepojenie zoznamu  
      if(L->Act == L->First)
        L->First = L->Act->lptr;  
      else 
        tmp->rptr = L->Act->lptr;
      L->Act->lptr->lptr = tmp;
      L->Act->lptr->rptr = L->Act;
      L->Act->lptr->data = val;
    }
  }
	return;
}

void DLCopy (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
  if(L->Act == NULL)
    DLError();
  else
    *val = L->Act->data;
	return;
}

void DLActualize (tDLList *L, int val) {
/*
** Pøepí¹e obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedìlá nic.
**/
  if(L->Act != NULL)
    L->Act->data = val;
	return;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na posledním prvku se seznam stane neaktivním.
**/
  if(L->Act != NULL){
    if(L->Act == L->Last)
      L->Act = NULL;
    else                   
      L->Act = L->Act->rptr;
  }
  return;
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na pøedchozí prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na prvním prvku se seznam stane neaktivním.
**/
  if(L->Act != NULL){
    if(L->Act == L->First)
      L->Act = NULL;
    else
      L->Act = L->Act->lptr;
  }
  return;
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním pøíkazem return.
**/
	return L->Act != NULL;
}

/* Konec c206.c*/

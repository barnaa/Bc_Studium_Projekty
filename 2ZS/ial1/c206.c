	
/* c206.c **********************************************************}
{* T�ma: Dvousm�rn� v�zan� line�rn� seznam
**
**                   N�vrh a referen�n� implementace: Bohuslav K�ena, ��jen 2001
**                            P�epracovan� do jazyka C: Martin Tu�ek, ��jen 2004
**                                            �pravy: Bohuslav K�ena, ��jen 2015
**
** Implementujte abstraktn� datov� typ dvousm�rn� v�zan� line�rn� seznam.
** U�ite�n�m obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datov� abstrakce reprezentov�n prom�nnou
** typu tDLList (DL znamen� Double-Linked a slou�� pro odli�en�
** jmen konstant, typ� a funkc� od jmen u jednosm�rn� v�zan�ho line�rn�ho
** seznamu). Definici konstant a typ� naleznete v hlavi�kov�m souboru c206.h.
**
** Va��m �kolem je implementovat n�sleduj�c� operace, kter� spolu
** s v��e uvedenou datovou ��st� abstrakce tvo�� abstraktn� datov� typ
** obousm�rn� v�zan� line�rn� seznam:
**
**      DLInitList ...... inicializace seznamu p�ed prvn�m pou�it�m,
**      DLDisposeList ... zru�en� v�ech prvk� seznamu,
**      DLInsertFirst ... vlo�en� prvku na za��tek seznamu,
**      DLInsertLast .... vlo�en� prvku na konec seznamu, 
**      DLFirst ......... nastaven� aktivity na prvn� prvek,
**      DLLast .......... nastaven� aktivity na posledn� prvek, 
**      DLCopyFirst ..... vrac� hodnotu prvn�ho prvku,
**      DLCopyLast ...... vrac� hodnotu posledn�ho prvku, 
**      DLDeleteFirst ... zru�� prvn� prvek seznamu,
**      DLDeleteLast .... zru�� posledn� prvek seznamu, 
**      DLPostDelete .... ru�� prvek za aktivn�m prvkem,
**      DLPreDelete ..... ru�� prvek p�ed aktivn�m prvkem, 
**      DLPostInsert .... vlo�� nov� prvek za aktivn� prvek seznamu,
**      DLPreInsert ..... vlo�� nov� prvek p�ed aktivn� prvek seznamu,
**      DLCopy .......... vrac� hodnotu aktivn�ho prvku,
**      DLActualize ..... p�ep�e obsah aktivn�ho prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal�� prvek seznamu,
**      DLPred .......... posune aktivitu na p�edchoz� prvek seznamu, 
**      DLActive ........ zji��uje aktivitu seznamu.
**
** P�i implementaci jednotliv�ch funkc� nevolejte ��dnou z funkc�
** implementovan�ch v r�mci tohoto p��kladu, nen�-li u funkce
** explicitn� uvedeno n�co jin�ho.
**
** Nemus�te o�et�ovat situaci, kdy m�sto leg�ln�ho ukazatele na seznam 
** p�ed� n�kdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodn� komentujte!
**
** Terminologick� pozn�mka: Jazyk C nepou��v� pojem procedura.
** Proto zde pou��v�me pojem funkce i pro operace, kter� by byly
** v algoritmick�m jazyce Pascalovsk�ho typu implemenov�ny jako
** procedury (v jazyce C procedur�m odpov�daj� funkce vracej�c� typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozorn�n� na to, �e do�lo k chyb�.
** Tato funkce bude vol�na z n�kter�ch d�le implementovan�ch operac�.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* glob�ln� prom�nn� -- p��znak o�et�en� chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L p�ed jeho prvn�m pou�it�m (tzn. ��dn�
** z n�sleduj�c�ch funkc� nebude vol�na nad neinicializovan�m seznamem).
** Tato inicializace se nikdy nebude prov�d�t nad ji� inicializovan�m
** seznamem, a proto tuto mo�nost neo�et�ujte. V�dy p�edpokl�dejte,
** �e neinicializovan� prom�nn� maj� nedefinovanou hodnotu.
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
** Zru�� v�echny prvky seznamu L a uvede seznam do stavu, v jak�m
** se nach�zel po inicializaci. Ru�en� prvky seznamu budou korektn�
** uvoln�ny vol�n�m operace free. 
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
** Vlo�� nov� prvek na za��tek seznamu L.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Vlo�� nov� prvek na konec seznamu L (symetrick� operace k DLInsertFirst).
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Nastav� aktivitu na prvn� prvek seznamu L.
** Funkci implementujte jako jedin� p��kaz (nepo��t�me-li return),
** ani� byste testovali, zda je seznam L pr�zdn�.
**/
  L->Act = L->First;
  return;
}

void DLLast (tDLList *L) {
/*
** Nastav� aktivitu na posledn� prvek seznamu L.
** Funkci implementujte jako jedin� p��kaz (nepo��t�me-li return),
** ani� byste testovali, zda je seznam L pr�zdn�.
**/
  L->Act = L->Last;
  return;
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prost�ednictv�m parametru val vr�t� hodnotu prvn�ho prvku seznamu L.
** Pokud je seznam L pr�zdn�, vol� funkci DLError().
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
** Prost�ednictv�m parametru val vr�t� hodnotu posledn�ho prvku seznamu L.
** Pokud je seznam L pr�zdn�, vol� funkci DLError().
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
** Zru�� prvn� prvek seznamu L. Pokud byl prvn� prvek aktivn�, aktivita 
** se ztr�c�. Pokud byl seznam L pr�zdn�, nic se ned�je.
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
** Zru�� posledn� prvek seznamu L. Pokud byl posledn� prvek aktivn�,
** aktivita seznamu se ztr�c�. Pokud byl seznam L pr�zdn�, nic se ned�je.
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
** Zru�� prvek seznamu L za aktivn�m prvkem.
** Pokud je seznam L neaktivn� nebo pokud je aktivn� prvek
** posledn�m prvkem seznamu, nic se ned�je.
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
** Zru�� prvek p�ed aktivn�m prvkem seznamu L .
** Pokud je seznam L neaktivn� nebo pokud je aktivn� prvek
** prvn�m prvkem seznamu, nic se ned�je.
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
** Vlo�� prvek za aktivn� prvek seznamu L.
** Pokud nebyl seznam L aktivn�, nic se ned�je.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Vlo�� prvek p�ed aktivn� prvek seznamu L.
** Pokud nebyl seznam L aktivn�, nic se ned�je.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Prost�ednictv�m parametru val vr�t� hodnotu aktivn�ho prvku seznamu L.
** Pokud seznam L nen� aktivn�, vol� funkci DLError ().
**/
  if(L->Act == NULL)
    DLError();
  else
    *val = L->Act->data;
	return;
}

void DLActualize (tDLList *L, int val) {
/*
** P�ep�e obsah aktivn�ho prvku seznamu L.
** Pokud seznam L nen� aktivn�, ned�l� nic.
**/
  if(L->Act != NULL)
    L->Act->data = val;
	return;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na n�sleduj�c� prvek seznamu L.
** Nen�-li seznam aktivn�, ned�l� nic.
** V�imn�te si, �e p�i aktivit� na posledn�m prvku se seznam stane neaktivn�m.
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
** Posune aktivitu na p�edchoz� prvek seznamu L.
** Nen�-li seznam aktivn�, ned�l� nic.
** V�imn�te si, �e p�i aktivit� na prvn�m prvku se seznam stane neaktivn�m.
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
** Je-li seznam L aktivn�, vrac� nenulovou hodnotu, jinak vrac� 0.
** Funkci je vhodn� implementovat jedn�m p��kazem return.
**/
	return L->Act != NULL;
}

/* Konec c206.c*/

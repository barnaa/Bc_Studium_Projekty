
/* c401.c: **********************************************************}
{* Téma: Rekurzivní implementace operací nad BVS
**                                         Vytvořil: Petr Přikryl, listopad 1994
**                                         Úpravy: Andrea Němcová, prosinec 1995
**                                                      Petr Přikryl, duben 1996
**                                                   Petr Přikryl, listopad 1997
**                                  Převod do jazyka C: Martin Tuček, říjen 2005
**                                         Úpravy: Bohuslav Křena, listopad 2009
**                                         Úpravy: Karel Masařík, říjen 2013
**                                         Úpravy: Radek Hranický, říjen 2014
**                                         Úpravy: Radek Hranický, listopad 2015
**
** Implementujte rekurzivním způsobem operace nad binárním vyhledávacím
** stromem (BVS; v angličtině BST - Binary Search Tree).
**
** Klíčem uzlu stromu je jeden znak (obecně jím může být cokoliv, podle
** čeho se vyhledává). Užitečným (vyhledávaným) obsahem je zde integer.
** Uzly s menším klíčem leží vlevo, uzly s větším klíčem leží ve stromu
** vpravo. Využijte dynamického přidělování paměti.
** Rekurzivním způsobem implementujte následující funkce:
**
**   BSTInit ...... inicializace vyhledávacího stromu
**   BSTSearch .... vyhledávání hodnoty uzlu zadaného klíčem
**   BSTInsert .... vkládání nové hodnoty
**   BSTDelete .... zrušení uzlu se zadaným klíčem
**   BSTHeight .... výpočet výšky stromu
**   BSTDispose ... zrušení celého stromu
**
** ADT BVS je reprezentován kořenovým ukazatelem stromu (typ tBSTNodePtr).
** Uzel stromu (struktura typu tBSTNode) obsahuje klíč (typu char), podle
** kterého se ve stromu vyhledává, vlastní obsah uzlu (pro jednoduchost
** typu int) a ukazatel na levý a pravý podstrom (LPtr a RPtr). Přesnou definici typů 
** naleznete v souboru c401.h.
**
** Pozor! Je třeba správně rozlišovat, kdy použít dereferenční operátor *
** (typicky při modifikaci) a kdy budeme pracovat pouze se samotným ukazatelem 
** (např. při vyhledávání). V tomto příkladu vám napoví prototypy funkcí.
** Pokud pracujeme s ukazatelem na ukazatel, použijeme dereferenci.
**/

#include "c401.h"
int solved;

void BSTInit (tBSTNodePtr *RootPtr) {
/*   -------
** Funkce provede počáteční inicializaci stromu před jeho prvním použitím.
**
** Ověřit, zda byl již strom předaný přes RootPtr inicializován, nelze,
** protože před první inicializací má ukazatel nedefinovanou (tedy libovolnou)
** hodnotu. Programátor využívající ADT BVS tedy musí zajistit, aby inicializace
** byla volána pouze jednou, a to před vlastní prací s BVS. Provedení
** inicializace nad neprázdným stromem by totiž mohlo vést ke ztrátě přístupu
** k dynamicky alokované paměti (tzv. "memory leak").
**	
** Všimněte si, že se v hlavičce objevuje typ ukazatel na ukazatel.	
** Proto je třeba při přiřazení přes RootPtr použít dereferenční operátor *.
** Ten bude použit i ve funkcích BSTDelete, BSTInsert a BSTDispose.
**/

	*RootPtr = NULL;
	
	return;
	
}	

int BSTSearch (tBSTNodePtr RootPtr, char K, int *Content)	{
/*  ---------
** Funkce vyhledá uzel v BVS s klíčem K.
**
** Pokud je takový nalezen, vrací funkce hodnotu TRUE a v proměnné Content se
** vrací obsah příslušného uzlu.´Pokud příslušný uzel není nalezen, vrací funkce
** hodnotu FALSE a obsah proměnné Content není definován (nic do ní proto
** nepřiřazujte).
**
** Při vyhledávání v binárním stromu bychom typicky použili cyklus ukončený
** testem dosažení listu nebo nalezení uzlu s klíčem K. V tomto případě ale
** problém řešte rekurzivním volání této funkce, přičemž nedeklarujte žádnou
** pomocnou funkci.
**/
	// Kluc nebol najdeny
	if(RootPtr == NULL)
		return FALSE;

	// Ak sa rovna kluc uzla s pozadovanym klucom tak sme ho nasli
	if(RootPtr->Key == K){
		*Content = RootPtr->BSTNodeCont;
		return TRUE;
	}
	else if(RootPtr->Key > K)
		// Kluc v uzle je vacsi ako pozadovany kluc - hladame vlavo
		return BSTSearch(RootPtr->LPtr, K, Content);
	else
		// Kluc v uzle je mensi ako pozadovany kluc - hladame vpravo
		return BSTSearch(RootPtr->RPtr, K, Content);

	// Sem to nikdy nedojde
	//return FALSE;
}


void BSTInsert (tBSTNodePtr* RootPtr, char K, int Content)	{	
/*   ---------
** Vloží do stromu RootPtr hodnotu Content s klíčem K.
**
** Pokud již uzel se zadaným klíčem ve stromu existuje, bude obsah uzlu
** s klíčem K nahrazen novou hodnotou. Pokud bude do stromu vložen nový
** uzel, bude vložen vždy jako list stromu.
**
** Funkci implementujte rekurzivně. Nedeklarujte žádnou pomocnou funkci.
**
** Rekurzivní implementace je méně efektivní, protože se při každém
** rekurzivním zanoření ukládá na zásobník obsah uzlu (zde integer).
** Nerekurzivní varianta by v tomto případě byla efektivnější jak z hlediska
** rychlosti, tak z hlediska paměťových nároků. Zde jde ale o školní
** příklad, na kterém si chceme ukázat eleganci rekurzivního zápisu.
**/

	// Strom este nema ziadny uzol, vkladame prvy uzol a inicializujeme ho
	if(*RootPtr == NULL){
			*RootPtr = malloc(sizeof(struct tBSTNode));
			if(*RootPtr == NULL){
  				fprintf(stderr, "Chyba alokacie pamate! Novy uzol nebol vytvoreny!\n");
				return;
			}
			(*RootPtr)->Key = K;
			(*RootPtr)->BSTNodeCont = Content;
			(*RootPtr)->LPtr = NULL;
			(*RootPtr)->RPtr = NULL;
			return;
	}

	// Ak sa kluc uzla rovna pozadovanemu klucu, tak sme ho nasli a aktualizujeme obsah
	if((*RootPtr)->Key == K)
		(*RootPtr)->BSTNodeCont = Content;
	else{
		tBSTNodePtr tmp = NULL;

		// Ak je hladany kluc mensi nez kluc uzla tak ideme vlavo, inak vpravo
		if((*RootPtr)->Key > K)
			tmp = (*RootPtr)->LPtr;
		else
			tmp = (*RootPtr)->RPtr;

		// Ak podstrom nie je prazdny tak hladame tam
		if(tmp != NULL)
			BSTInsert(&tmp, K, Content);
		else{
			// Inak vkladame novy prvok
			tmp = malloc(sizeof(struct tBSTNode));
			if(tmp == NULL){
  				fprintf(stderr, "Chyba alokacie pamate! Novy uzol nebol vytvoreny!\n");
				return;
			}
			tmp->Key = K;
			tmp->BSTNodeCont = Content;
			tmp->LPtr = NULL;
			tmp->RPtr = NULL;

			// Napokon spojime rodicovsky uzol s novo vytvorenym uzlom
			if((*RootPtr)->Key > tmp->Key)
				(*RootPtr)->LPtr = tmp;
			else
				(*RootPtr)->RPtr = tmp;
		}
	}
	
	return;
	
}

void ReplaceByRightmost (tBSTNodePtr PtrReplaced, tBSTNodePtr *RootPtr) {
/*   ------------------
** Pomocná funkce pro vyhledání, přesun a uvolnění nejpravějšího uzlu.
**
** Ukazatel PtrReplaced ukazuje na uzel, do kterého bude přesunuta hodnota
** nejpravějšího uzlu v podstromu, který je určen ukazatelem RootPtr.
** Předpokládá se, že hodnota ukazatele RootPtr nebude NULL (zajistěte to
** testováním před volání této funkce). Tuto funkci implementujte rekurzivně. 
**
** Tato pomocná funkce bude použita dále. Než ji začnete implementovat,
** přečtěte si komentář k funkci BSTDelete(). 
**/

 	if((*RootPtr)->RPtr == NULL){
	  	// Toto nastane, len ak sme zavolali tuto funkciu na uzol, ktory nema pravy podstrom
		PtrReplaced->Key = (*RootPtr)->Key;
		PtrReplaced->BSTNodeCont = (*RootPtr)->BSTNodeCont;
	
 		tBSTNodePtr tmp = (*RootPtr)->LPtr;
 		free(*RootPtr);
 		PtrReplaced->LPtr = tmp;
 		return;
	}
	else if((*RootPtr)->RPtr->RPtr == NULL){
		// Uzol napravo od *RootPtr je ten najpravejsi, teda presunieme data
		PtrReplaced->Key = (*RootPtr)->RPtr->Key;
		PtrReplaced->BSTNodeCont = (*RootPtr)->RPtr->BSTNodeCont;

		// Odstranime dany uzol a nahradime ho s uzlom nalavo od neho
		tBSTNodePtr tmp = (*RootPtr)->RPtr->LPtr;
		free((*RootPtr)->RPtr);
		(*RootPtr)->RPtr = tmp;
	}
	// Inak sa posuvame doprava
	else ReplaceByRightmost(PtrReplaced, &((*RootPtr)->RPtr));

	return;
	
}

void BSTDelete (tBSTNodePtr *RootPtr, char K){
/*   ---------
** Zruší uzel stromu, který obsahuje klíč K.
**
** Pokud uzel se zadaným klíčem neexistuje, nedělá funkce nic. 
** Pokud má rušený uzel jen jeden podstrom, pak jej zdědí otec rušeného uzlu.
** Pokud má rušený uzel oba podstromy, pak je rušený uzel nahrazen nejpravějším
** uzlem levého podstromu. Pozor! Nejpravější uzel nemusí být listem.
**
** Tuto funkci implementujte rekurzivně s využitím dříve deklarované
** pomocné funkce ReplaceByRightmost.
**/
	// Nie je co rusit, vraciame sa
	if(*RootPtr == NULL)
		return;
	
	if((*RootPtr)->Key == K){
		// Nasla sa zhoda hladaneho kluca s klucom aktualneho uzlu, ideme ho zrusit

		if((*RootPtr)->LPtr == NULL && (*RootPtr)->RPtr == NULL){
			// Nema ani jeden podstrom a nebol predtym odstraneny, cize je to koren celeho BVS
			free(*RootPtr);
			*RootPtr = NULL;
			return;
		}

		tBSTNodePtr tmp = NULL;
		if((*RootPtr)->RPtr == NULL)          // Ma lavy podstrom
		  tmp = (*RootPtr)->LPtr;
		else if((*RootPtr)->LPtr == NULL)     // Ma pravy podstrom
		  tmp = (*RootPtr)->RPtr;

		if(tmp == NULL)		
		  // Ma oba podstromy, uzol nahradzujeme najpravejsim uzlom laveho podstromu
		  ReplaceByRightmost(*RootPtr, &((*RootPtr)->LPtr));
		else{
		  // Ma jeden podstrom, presunieme udaje z korenoveho uzlu toho podstromu do
		  // aktualneho, "odstranovaneho" uzlu a korenovy uzol podstromu uvolnime
		  (*RootPtr)->Key = tmp->Key;
		  (*RootPtr)->BSTNodeCont = tmp->BSTNodeCont;
		  (*RootPtr)->LPtr = tmp->LPtr;
		  (*RootPtr)->RPtr = tmp->RPtr;
		  free(tmp);
		}
		return;
	}
	else{
		// Zhoda sa nenasla, hladame v patricnom podstrome 
		tBSTNodePtr tmp = NULL;
		if((*RootPtr)->Key > K)
			tmp = (*RootPtr)->LPtr;
		else
			tmp = (*RootPtr)->RPtr;

		if(tmp == NULL) // Podstrom je prazdny, uzol nebol najdeny
			return;

		// Ak korenovy uzol daneho podstromu je prvok, ktory mame odstranit a
		// nema ziadne podstromy tak ho odstranime hned, aby sme mohli nastavit
		// NULL hodnotu do patricneho ukazatela v *RootPtr uzle
		if((tmp->Key == K) && (tmp->LPtr == NULL && tmp->RPtr == NULL)){
			free(tmp);
			if((*RootPtr)->Key > K)
				(*RootPtr)->LPtr = NULL;
			else
				(*RootPtr)->RPtr = NULL;
		}
		else BSTDelete(&tmp, K);
	}
	
	return;
} 

int BSTHeight (tBSTNodePtr NodePtr, bool IsRoot) {	
/*   ----------
** Vypočítá výšku BVS. Výška stromu délka (počet hran) nejdelší cesty
** od kořene k listu. Vzhledem k rekurzivní implementaci je třeba rozlišit,
** zda funkci voláme pro samotný kořen stromu nebo rekurzivně pro některý
** z ostatních uzlů.
** Výpočet výšky stromu se tedy bude provádět voláním:
**   BSTHeight(ukazatel_na_kořen, TRUE)
**
** Návratová hodnota je výška stromu. Výška prázdného stromu však není definována.
** V případě prázdného stromu bude funkce vracet hodnotu -1. 
** 
** Tuto funkci implementujte bez deklarování pomocné funkce.
**/
	// Strom je prazdny
	int len = -1;

	if(NodePtr != NULL){
		int llen = BSTHeight(NodePtr->LPtr, FALSE);
		int rlen = BSTHeight(NodePtr->RPtr, FALSE);

		// Porovnavame vysky podstromov
		if(llen > rlen)
			len = llen;
		else
			len = rlen;

		// Oba podstromy su prazdne
		if(len == -1)
			len = 0;
	}

	// Nevraciame vysku zvysenu o 1, kedze uz nad Root uzlom nie je dalsia hrana
	if(IsRoot)
		return len;
	
	// Vratime najvacsiu vysku podstromov uzlu zvysenu o 1 (tj. hrana k tomuto uzlu)
	return len+1;
}

void BSTDispose (tBSTNodePtr *RootPtr) {	
/*   ----------
** Zruší celý binární vyhledávací strom a korektně uvolní paměť.
**
** Po zrušení se bude BVS nacházet ve stejném stavu, jako se nacházel po
** inicializaci. Tuto funkci implementujte rekurzivně bez deklarování pomocné
** funkce.
**/
	
	// Je to NULL, nie je tu co rusit
	if(*RootPtr == NULL)
		return;

	// Zrusime lavy podstrom
	if((*RootPtr)->LPtr != NULL)
		BSTDispose(&((*RootPtr)->LPtr));

	// Zrusime pravy podstrom
	if((*RootPtr)->RPtr != NULL)
		BSTDispose(&((*RootPtr)->RPtr));

	// Uzol nema podstrom, mozeme ho bezpecne uvolnit
	free(*RootPtr);
	*RootPtr = NULL;

	return;
}

/* konec c401.c */


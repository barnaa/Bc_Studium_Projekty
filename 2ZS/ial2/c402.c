
/* c402.c: ********************************************************************}
{* Téma: Nerekurzivní implementace operací nad BVS 
**                                     Implementace: Petr Přikryl, prosinec 1994
**                                           Úpravy: Petr Přikryl, listopad 1997
**                                                     Petr Přikryl, květen 1998
**			  	                        Převod do jazyka C: Martin Tuček, srpen 2005
**                                         Úpravy: Bohuslav Křena, listopad 2009
**                                         Úpravy: Karel Masařík, říjen 2013
**                                         Úpravy: Radek Hranický, říjen 2014
**                                         Úpravy: Radek Hranický, listopad 2015
**
** S využitím dynamického přidělování paměti, implementujte NEREKURZIVNĚ
** následující operace nad binárním vyhledávacím stromem (předpona BT znamená
** Binary Tree a je u identifikátorů uvedena kvůli možné kolizi s ostatními
** příklady):
**
**     BTInit .......... inicializace stromu
**     BTInsert ........ nerekurzivní vložení nového uzlu do stromu
**     BTPreorder ...... nerekurzivní průchod typu pre-order
**     BTInorder ....... nerekurzivní průchod typu in-order
**     BTPostorder ..... nerekurzivní průchod typu post-order
**     BTHeight ........ výpočet výšky stromu
**     BTDisposeTree ... zruš všechny uzly stromu
**
** U všech funkcí, které využívají některý z průchodů stromem, implementujte
** pomocnou funkci pro nalezení nejlevějšího uzlu v podstromu. Tyto pomocné
** funkce jsou:
**
**     Leftmost_Preorder
**     Leftmost_Inorder
**     Leftmost_Postorder
**
** Ve vaší implementaci si můžete definovat pomocné zásobníky pro uložení
** ukazetelů na uzly stromu (tStackP)
** nebo pro uložení booleovských hodnot TRUE/FALSE (tStackB).
** Pro práci s pomocnými zásobníky můžete použít následující funkce:
**
**     SInitP / SInitB ....... inicializace zásobníku
**     SPushP / SPushB ....... vložení prvku na vrchol zásobníku
**     SPopP / SPopB ......... odstranění prvku z vrcholu zásobníku
**     STopP / STopB ......... získání hodonty prvku na vrcholu zásobníku
**     STopPopP / STopPopB ... kombinace předchozích dvou funkcí 
**     SSizeP / SSizeB ....... zjištění počtu prvků v zásobníku
**     SEmptyP / SEmptyB ..... zjištění, zda je zásobník prázdný 
** 
** Pomocné funkce pro práci ze zásobníky je zakázáno upravovat!
** 
** Přesné definice typů naleznete v souboru c402.h. Uzel stromu je typu tBTNode,
** ukazatel na něj je typu tBTNodePtr. Jeden uzel obsahuje položku int Cont,
** která současně slouží jako užitečný obsah i jako vyhledávací klíč 
** a ukazatele na levý a pravý podstrom (LPtr a RPtr).
**
** Příklad slouží zejména k procvičení nerekurzivních zápisů algoritmů
** nad stromy. Než začnete tento příklad řešit, prostudujte si důkladně
** principy převodu rekurzivních algoritmů na nerekurzivní. Programování
** je především inženýrská disciplína, kde opětné objevování Ameriky nemá
** místo. Pokud se Vám zdá, že by něco šlo zapsat optimálněji, promyslete
** si všechny detaily Vašeho řešení. Povšimněte si typického umístění akcí
** pro různé typy průchodů. Zamyslete se nad modifikací řešených algoritmů
** například pro výpočet počtu uzlů stromu, počtu listů stromunebo pro
** vytvoření zrcadlového obrazu stromu (pouze popřehazování ukazatelů
** bez vytváření nových uzlů a rušení starých).
**
** Při průchodech stromem použijte ke zpracování uzlu funkci BTWorkOut().
** Pro zjednodušení práce máte předem připraveny zásobníky pro hodnoty typu
** bool a tBTNodePtr. Pomocnou funkci BTWorkOut ani funkce pro práci
** s pomocnými zásobníky neupravujte 
** Pozor! Je třeba správně rozlišovat, kdy použít dereferenční operátor *
** (typicky při modifikaci) a kdy budeme pracovat pouze se samotným ukazatelem 
** (např. při vyhledávání). V tomto příkladu vám napoví prototypy funkcí.
** Pokud pracujeme s ukazatelem na ukazatel, použijeme dereferenci.
**/

#include "c402.h"
int solved;

void BTWorkOut (tBTNodePtr Ptr)		{
/*   ---------
** Pomocná funkce, kterou budete volat při průchodech stromem pro zpracování
** uzlu určeného ukazatelem Ptr. Tuto funkci neupravujte.
**/
			
	if (Ptr==NULL) 
    printf("Chyba: Funkce BTWorkOut byla volána s NULL argumentem!\n");
  else 
    printf("Výpis hodnoty daného uzlu> %d\n",Ptr->Cont);
}
	
/* -------------------------------------------------------------------------- */
/*
** Funkce pro zásobník hotnot typu tBTNodePtr. Tyto funkce neupravujte.
**/

void SInitP (tStackP *S)  
/*   ------
** Inicializace zásobníku.
**/
{
	S->top = 0;  
}	

void SPushP (tStackP *S, tBTNodePtr ptr)
/*   ------
** Vloží hodnotu na vrchol zásobníku.
**/
{ 
                 /* Při implementaci v poli může dojít k přetečení zásobníku. */
  if (S->top==MAXSTACK) 
    printf("Chyba: Došlo k přetečení zásobníku s ukazateli!\n");
  else {  
		S->top++;  
		S->a[S->top]=ptr;
	}
}	

tBTNodePtr STopPopP (tStackP *S)
/*         --------
** Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
**/
{
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
		return(NULL);	
	}
	else {
		return (S->a[S->top--]);
	}
}

tBTNodePtr STopP (tStackP *S)
/*         --------
** Vrátí hodnotu prvku na vrcholu zásobníku
**/
{
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
		return(NULL);	
	}	
	else {
		return (S->a[S->top]);
	}	
}

void SPopP (tStackP *S)
/*         --------
** Odstraní prvek z vrcholu zásobníku
**/
{
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
	}	
	else {
		S->top--;
	}	
}

int SSizeP (tStackP *S) {
/*   -------
** Vrátí počet prvků v zásobníku
**/
  return(S->top);
}

bool SEmptyP (tStackP *S)
/*   -------
** Je-li zásobník prázdný, vrátí hodnotu true.
**/
{
  return(S->top==0);
}	

/* -------------------------------------------------------------------------- */
/*
** Funkce pro zásobník hotnot typu bool. Tyto funkce neupravujte.
*/

void SInitB (tStackB *S) {
/*   ------
** Inicializace zásobníku.
**/

	S->top = 0;  
}	

void SPushB (tStackB *S,bool val) {
/*   ------
** Vloží hodnotu na vrchol zásobníku.
**/
                 /* Při implementaci v poli může dojít k přetečení zásobníku. */
	if (S->top==MAXSTACK) 
		printf("Chyba: Došlo k přetečení zásobníku pro boolean!\n");
	else {
		S->top++;  
		S->a[S->top]=val;
	}	
}

bool STopPopB (tStackB *S) {
/*   --------
** Odstraní prvek z vrcholu zásobníku a současně vrátí jeho hodnotu.
**/
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0) {
		printf("Chyba: Došlo k podtečení zásobníku pro boolean!\n");
		return(NULL);	
	}	
	else {  
		return(S->a[S->top--]); 
	}	
}

bool STopB (tStackB *S)
/*         --------
** Vrátí hodnotu prvku na vrcholu zásobníku
**/
{
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
		return(NULL);	
	}	
	else {
		return (S->a[S->top]);
	}	
}

void SPopB (tStackB *S)
/*         --------
** Odstraní prvek z vrcholu zásobníku
**/
{
                            /* Operace nad prázdným zásobníkem způsobí chybu. */
	if (S->top==0)  {
		printf("Chyba: Došlo k podtečení zásobníku s ukazateli!\n");
	}	
	else {
		S->top--;
	}	
}

int SSizeB (tStackB *S) {
/*   -------
** Vrátí počet prvků v zásobníku
**/
  return(S->top);
}

bool SEmptyB (tStackB *S) {
/*   -------
** Je-li zásobník prázdný, vrátí hodnotu true.
**/
  return(S->top==0);
}

/* -------------------------------------------------------------------------- */
/*
** Následuje jádro domácí úlohy - funkce, které máte implementovat. 
*/

void BTInit (tBTNodePtr *RootPtr)	{
/*   ------
** Provede inicializaci binárního vyhledávacího stromu.
**
** Inicializaci smí programátor volat pouze před prvním použitím binárního
** stromu, protože neuvolňuje uzly neprázdného stromu (a ani to dělat nemůže,
** protože před inicializací jsou hodnoty nedefinované, tedy libovolné).
** Ke zrušení binárního stromu slouží procedura BTDisposeTree.
**	
** Všimněte si, že zde se poprvé v hlavičce objevuje typ ukazatel na ukazatel,	
** proto je třeba při práci s RootPtr použít dereferenční operátor *.
**/
	*RootPtr = NULL;
	
	return;
}

void BTInsert (tBTNodePtr *RootPtr, int Content) {
/*   --------
** Vloží do stromu nový uzel s hodnotou Content.
**
** Z pohledu vkládání chápejte vytvářený strom jako binární vyhledávací strom,
** kde uzly s hodnotou menší než má otec leží v levém podstromu a uzly větší
** leží vpravo. Pokud vkládaný uzel již existuje, neprovádí se nic (daná hodnota
** se ve stromu může vyskytnout nejvýše jednou). Pokud se vytváří nový uzel,
** vzniká vždy jako list stromu. Funkci implementujte nerekurzivně.
**/
	if(*RootPtr == NULL){
		// Dostali sme NULL pointer, toto nastane ak je strom prazdny

		// Tvorime uzol a nastavujeme hodnoty premennych v nom
		*RootPtr = malloc(sizeof(struct tBTNode));
		if(*RootPtr == NULL){
			fprintf(stderr, "Chyba alokacie pamate! Novy uzol nebol vlozeny!\n");
			return;
		}
		(*RootPtr)->Cont = Content;
		(*RootPtr)->LPtr = NULL;
		(*RootPtr)->RPtr = NULL;
		return;
	}
	else{
		// Tvorime si pomocne premenne...
		tBTNodePtr tmp = *RootPtr, prev = NULL;

		// ... a prechadzame stromom, kym nenajdeme uzol s pozadovanym klucom
		// alebo kym nenarazime na koniec, pricom si uchovavame predosly uzol
		while(tmp != NULL){
			prev = tmp;
			if(tmp->Cont == Content)
				return;
			else if(tmp->Cont > Content)
				tmp = tmp->LPtr;
			else
				tmp = tmp->RPtr;
		}

		// Uzol sme nenasli, tak ho vytvorime
		tmp = malloc(sizeof(struct tBTNode));
		if(tmp == NULL){
			fprintf(stderr, "Chyba alokacie pamate! Novy uzol nebol vlozeny!\n");
			return;
		}
		tmp->Cont = Content;
		tmp->LPtr = NULL;
		tmp->RPtr = NULL;

		// V predoslom uzle nastavime patricny ukazatel na novo vytvoreny uzol
		if(prev->Cont > Content)
			prev->LPtr = tmp;
		else
			prev->RPtr = tmp;
	}
	
	return;
}

/*                                  PREORDER                                  */

void Leftmost_Preorder (tBTNodePtr ptr, tStackP *Stack)	{
/*   -----------------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Preorder navštívené uzly zpracujeme voláním funkce BTWorkOut()
** a ukazatele na ně is uložíme do zásobníku.
**/
	// Ideme stale dolava a po ceste si pushujeme ukazatele a spracovavame uzly
	while(ptr->LPtr != NULL){
		SPushP(Stack, ptr);
		BTWorkOut(ptr);
		ptr = ptr->LPtr;
	}

	// Spracujeme aj ten najlavejsi uzol...
	BTWorkOut(ptr);
	SPushP(Stack, ptr);
	
	return;
}

void BTPreorder (tBTNodePtr RootPtr)	{
/*   ----------
** Průchod stromem typu preorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Preorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
** realizujte jako volání funkce BTWorkOut(). 
**/

	// Ideme na najlavejsi prvok, ak nema podstromy tak popneme predosly uzol a
	// ideme dalej na najlavejsi prvok praveho podstromu

	// Prazdny strom, nerobime nic
	if(RootPtr == NULL)
		return;
	
	// Vytvorime si zasobnik a inicializujeme ho
	tStackP *Stack = malloc(sizeof(tStackP));
	if(Stack == NULL){
		fprintf(stderr, "Chyba alokacie pamate! Nedalo sa naalokovat zasobnik!\n");
		return;
	}
	SInitP(Stack);
	
	// Vytvorime si pomocny ukazatel a na zaciatku ideme na najlavejsi uzol
	tBTNodePtr tmp = RootPtr;
	Leftmost_Preorder(tmp, Stack);
	tmp = STopPopP(Stack); // Popneme si adresu najlavejsieho uzlu

	// Dookola prechadzame strom kym sa nevyprazdni zasobnik alebo kym je pravy
	// podstrom neprazdny
	while(!SEmptyP(Stack) || tmp->RPtr != NULL){
		if(tmp->RPtr != NULL){
			// Existuje pravy podstrom, tak ho spracujeme
    		tmp = tmp->RPtr;
			Leftmost_Preorder(tmp, Stack);
		}
		tmp = STopPopP(Stack);
	}
	
	free(Stack);
	return;
}


/*                                  INORDER                                   */ 

void Leftmost_Inorder(tBTNodePtr ptr, tStackP *Stack)		{
/*   ----------------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Inorder ukládáme ukazatele na všechny navštívené uzly do
** zásobníku. 
**/
	
	// Ideme stale dolava a po ceste si pushujeme ukazatele
	while(ptr->LPtr != NULL){
		SPushP(Stack, ptr);
		ptr = ptr->LPtr;
	}

	// Pushneme aj ten najlavejsi uzol a rovno ho spracujeme
	SPushP(Stack, ptr);
	BTWorkOut(ptr);
	
	return;
	
}

void BTInorder (tBTNodePtr RootPtr)	{
/*   ---------
** Průchod stromem typu inorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Inorder a zásobníku ukazatelů. Zpracování jednoho uzlu stromu
** realizujte jako volání funkce BTWorkOut(). 
**/
	// Prvok vypiseme vzdy, ked nema lavy podstrom, alebo ked bol popnuty zo zasobniku

	// Prazdny strom, nerobime nic
	if(RootPtr == NULL)
		return;
	
	// Vytvorime si zasobnik a inicializujeme ho
	tStackP *Stack = malloc(sizeof(tStackP));
	if(Stack == NULL){
		fprintf(stderr, "Chyba alokacie pamate! Nedalo sa naalokovat zasobnik!\n");
		return;
	}
	SInitP(Stack);
	
	// Vytvorime si pomocny ukazatel a na zaciatku ideme na najlavejsi uzol
	tBTNodePtr tmp = RootPtr;
	Leftmost_Inorder(tmp, Stack);
	tmp = STopPopP(Stack); // Popneme si najlavejsi uzol

	// Dookola prechadzame strom kym sa nevyprazdni zasobnik alebo kym je pravy
	// podstrom neprazdny
	while(!SEmptyP(Stack) || tmp->RPtr != NULL){
		if(tmp->RPtr != NULL){
			// Existuje pravy podstrom, tak ho spracujeme
    		tmp = tmp->RPtr;
			Leftmost_Inorder(tmp, Stack);
			tmp = STopPopP(Stack); // Popneme si najlavejsi uzol tohto podstromu
		}
		else{
			tmp = STopPopP(Stack);
			BTWorkOut(tmp);
		}
	}
	
	free(Stack);
	return;
}

/*                                 POSTORDER                                  */ 

void Leftmost_Postorder (tBTNodePtr ptr, tStackP *StackP, tStackB *StackB) {
/*           --------
** Jde po levě větvi podstromu, dokud nenarazí na jeho nejlevější uzel.
**
** Při průchodu Postorder ukládáme ukazatele na navštívené uzly do zásobníku
** a současně do zásobníku bool hodnot ukládáme informaci, zda byl uzel
** navštíven poprvé a že se tedy ještě nemá zpracovávat. 
**/

	// Ideme stale dolava a po ceste si pushujeme ukazatele
	while(ptr->LPtr != NULL){
		SPushP(StackP, ptr);
		SPushB(StackB, FALSE);
		ptr = ptr->LPtr;
	}

	// Pushneme si aj najlavejsi uzol
	SPushP(StackP, ptr);
	SPushB(StackB, FALSE);
	
	return;
}

void BTPostorder (tBTNodePtr RootPtr)	{
/*           -----------
** Průchod stromem typu postorder implementovaný nerekurzivně s využitím funkce
** Leftmost_Postorder, zásobníku ukazatelů a zásobníku hotdnot typu bool.
** Zpracování jednoho uzlu stromu realizujte jako volání funkce BTWorkOut(). 
**/

	// Prazdny strom, nerobime nic
	if(RootPtr == NULL)
		return;
	
	// Vytvorime si zasobniky a inicializujeme ich
	tStackP *StackP = malloc(sizeof(tStackP));
	if(StackP == NULL){
		fprintf(stderr, "Chyba alokacie pamate! Nedalo sa naalokovat zasobnik!\n");
		return;
	}
	SInitP(StackP);

	tStackB *StackB = malloc(sizeof(tStackB));
	if(StackB == NULL){
		fprintf(stderr, "Chyba alokacie pamate! Nedalo sa naalokovat zasobnik!\n");
		return;
	}
	SInitB(StackB);
	

	// Vytvorime si pomocny ukazatel a na zaciatku ideme na najlavejsi uzol
	tBTNodePtr tmp = RootPtr;
	Leftmost_Postorder(tmp, StackP, StackB);
	tmp = STopPopP(StackP);

	// Dookola prechadzame strom kym sa nevyprazdni zasobnik alebo kym je pravy
	// podstrom neprazdny
	while(!SEmptyB(StackB)){
		if(tmp->RPtr != NULL && !STopB(StackB)){
			// Existuje pravy podstrom a nebol este navstiveny, tak ho spracujeme
			// Popneme FALSE zo stacku a pushneme TRUE
			SPopB(StackB);
			SPushB(StackB, TRUE);
			SPushP(StackP, tmp);
    		tmp = tmp->RPtr;
			Leftmost_Postorder(tmp, StackP, StackB);
			tmp = STopPopP(StackP);
		}
		else{ // Vraciame sa vyssie, kedze sme uz nenasli pravy podstrom, spracujeme tento uzol
			BTWorkOut(tmp);
			SPopB(StackB);
			// Podmienka je pravdiva ak sa jedna o RootPtr, teda dalej nejdeme
			if(SEmptyB(StackB)) 
				break;
			tmp = STopPopP(StackP);
		}
	}

	
	free(StackP);
	free(StackB);
	return;

}


int BTHeight (tBTNodePtr RootPtr) {	
/*   ----------
** Vypočítá výšku BVS bez použití rekurze
**
** Návratová hodnota je výška stromu. Funkci implementujte nerekurzivně
** bez deklarování jakékoli další pomocné funkce, která není v zadání.
** Využijte pomocných zásobníků. Je doporučeno použít jeden ze zásobníků
** pro průběžné ukládání cesty od kořene stromu. Počet uzlů na takovéto
** cestě můžete zjistit použitím funkce SSizeP. Výška stromu je rovna
** délce (počtu hran) nejdelší cesty  od kořene k lisu.
**
** Výška prázdného stromu však není definována. V případě prázdného stromu
** bude funkce vracet hodnotu -1.  
**/
	// Strom je prazdny, vraciame -1
	if(RootPtr == NULL)
		return -1;

	// Vytvorime si zasobnik a inicializujeme ho
	tStackP *Stack = malloc(sizeof(tStackP));
	if(Stack == NULL){
		fprintf(stderr, "Chyba alokacie pamate! Nedalo sa naalokovat zasobnik!\n");
		return -1;
	}
	SInitP(Stack);

	// Premenna do ktorej budeme priebezne ukladat maximalnu najdenu vysku stromu
	int maxlen = 0;

	// Pomocny pointer na aktualne spracovavany uzol
	tBTNodePtr tmp = RootPtr;

	// Prechadzame strom kym je nejaky pointer na zasobniku, alebo ak aktualne
	// spracovavany uzol ma podstrom
	while(!SEmptyP(Stack) || tmp->RPtr != NULL || tmp->LPtr != NULL){
		if(tmp->LPtr != NULL){
			// Ak je to mozne, tak ideme dolava
			SPushP(Stack, tmp);
			tmp = tmp->LPtr;
		}
		else{
			if(tmp->RPtr != NULL){
				// Ak nie je mozne ist dolava tak ideme doprava ak je to mozne
				SPushP(Stack, tmp);
				tmp = tmp->RPtr;
			}
			else{
				// Inak sme narazili na list, cize sa uz mozeme len vratit

				// Najprv porovname dlzku aktualnej cesty, ak je vacsia nez
				// doterajsie maximum tak si ju ulozime
				if(maxlen < SSizeP(Stack))
					maxlen = SSizeP(Stack);

				// Inak sa vraciame kym posledny navstiveny uzol je pravym
				// podstromom rodicovskeho uzlu
				tBTNodePtr prev = STopPopP(Stack);
				while(prev->RPtr == tmp || prev->RPtr == NULL){
					if(SEmptyP(Stack)){
						free(Stack);
						return maxlen;
					}
					tmp = prev;
					prev = STopPopP(Stack);
				}
				SPushP(Stack, prev);
				tmp = prev->RPtr;
			}
		}
	}
	
	free(Stack);
	return maxlen;
}


void BTDisposeTree (tBTNodePtr *RootPtr)	{
/*   -------------
** Zruší všechny uzly stromu a korektně uvolní jimi zabranou paměť.
**
** Funkci implementujte nerekurzivně s využitím zásobníku ukazatelů.
**/

	// Prazdny strom, nerobime nic
	if(*RootPtr == NULL)
		return;
	
	// Vytvorime si zasobnik a inicializujeme ho
	tStackP *Stack = malloc(sizeof(tStackP));
	if(Stack == NULL){
		fprintf(stderr, "Chyba alokacie pamate! Nedalo sa naalokovat zasobnik!\n");
		return;
	}
	SInitP(Stack);
	
	// Pomocny pointer na aktualne spracovavany uzol
	tBTNodePtr tmp = *RootPtr;

	// Prechadzame strom kym je nejaky pointer na zasobniku, alebo ak aktualne
	// spracovavany uzol ma podstrom
	while(!SEmptyP(Stack) || tmp->RPtr != NULL || tmp->LPtr != NULL){
		if(tmp->LPtr != NULL){
			// Ak je to mozne, tak ideme dolava
			SPushP(Stack, tmp);
			tmp = tmp->LPtr;
		}
		else{
			if(tmp->RPtr != NULL){
				// Ak nie je mozne ist dolava tak ideme doprava ak je to mozne
				SPushP(Stack, tmp);
				tmp = tmp->RPtr;
			}
			else{
				// Inak sme nasli list, ktory mozeme bezpecne uvolnit bez leaku
				// Popneme si zo zasobnika predosly uzol a tento uzol uvolnime
				tBTNodePtr rem = tmp;
				tmp = STopPopP(Stack);
				if(tmp->LPtr == rem)
					tmp->LPtr = NULL;
				else tmp->RPtr = NULL;
				free(rem);
			}
		}
	}

	// Uvolnime posledny, korenovy uzol
	free(*RootPtr);
	*RootPtr = NULL;
	
	free(Stack);
	return;
}

/* konec c402.c */


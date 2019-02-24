#include <stdio.h>

#include <listx.h>
#include <const.h>
#include <types_rikaya.h>
#include "pcb.h"

pcb_t pcbFree_table[MAXPROC];

struct list_head pcbFree_h;

void initPcbs(){
	INIT_LIST_HEAD(&pcbFree_h); //inizializza i campi di una struttura già esistente
	for(int i = 0; i < MAXPROC; i++){
		list_add_tail(&(pcbFree_table[i].p_next), &pcbFree_h);
	}
}

void freePcb(pcb_t *p){
	list_add_tail(&p->p_next, &pcbFree_h);
}

pcb_t *allocPcb(){
	if(!list_empty(&pcbFree_h)){
	
		pcb_t* punt;
		//punt è l'indirizzo dell'elemento puntato dalla sentinella estratto da list_head.
		punt = container_of(pcbFree_h.next, pcb_t, p_next);

		punt->p_parent = NULL;
		INIT_LIST_HEAD(&(punt->p_child));
		INIT_LIST_HEAD(&(punt->p_sib));
		INIT_LIST_HEAD(&(punt->p_next));
		//inizializzazione campi che costituiscono state_t p_s
		punt->p_s.entry_hi = 0;
		punt->p_s.cause = 0;
		punt->p_s.status = 0;
		punt->p_s.pc_epc = 0;

		for(int i = 0; i < STATE_GPR_LEN; i++){
			punt->p_s.gpr[i] = 0;
		}
		punt->p_s.hi = 0;
		punt->p_s.lo = 0;

		list_del(pcbFree_h.next);

		return punt;
	}

	return NULL;
}

//inizializza lista dei PCB inizializzando la sentinella.
pcb_t* mkEmptyProcQ(struct list_head* head){
	
	INIT_LIST_HEAD(head);
	
	return container_of(head, pcb_t, p_next);
}

//restituisco true se la lista puntata da head è vuota, false altrimenti.
int emptyProcQ(struct list_head* head){
	if(list_empty(head)) return TRUE;
	return FALSE;
}

void insertProcQ(struct list_head* head, pcb_t* p){
	if(!(list_empty(head))){
		int prior = p->priority;
		pcb_t *temp;
		struct list_head* pos;
		list_for_each(pos, head){
			temp = container_of(head, pcb_t, p_next);
			if (temp->priority < prior){ //se la prorità successiva è minore
				__list_add(&p->p_next, head->prev, head); //inserisco tra prev e next.
				return;}
			}
	}
	list_add_tail(&p->p_next, head); //se la lista è vuota.
}

pcb_t* headProcQ(struct list_head* head){
	if (!list_empty(head)){
		return container_of(head->next,pcb_t,p_next);
	}

	return NULL;
}

pcb_t* removeProcQ(struct list_head* head){
	pcb_t *temp;
	temp = container_of(head->next, pcb_t, p_next);
	if(temp != NULL){
		list_del(head->next);
		return temp;
	}

	return NULL;
}

pcb_t* outProcQ(struct list_head* head, pcb_t *p){
	struct list_head *pos;
	pcb_t *temp;
	list_for_each(pos, head){
		temp = container_of(head, pcb_t, p_next);
		if(temp  == p){
			list_del(head->next);
			return temp;
		}
	}

	return NULL;
}

/*ipotizzando che la lista dei figli sia una lista bidirezionale con sentinella
per verificare che la lista dei figli sia vuota basterà verificare che la sentinella punti a se stessa*/
int emptyChild(pcb_t *p){
	return list_empty(&p->p_child);
}
/*supponendo sempre che la lista dei figli sia una lista
bidirezionale basterà andare ad inserire in coda il nuovo
figlio ed creare l'associazione dal figlio al padre*/
void insertChild(pcb_t *prnt,pcb_t *p){

		p->p_parent = prnt;
		list_add_tail(&p->p_sib, &prnt->p_child);
}

pcb_t *removeChild(pcb_t *p){
	if(!emptyChild(p) &&  p != NULL){
		struct pcb_t *pos;
		/*vado a creare una variabile d'appoggio per sapere qual'è il list head
		di p->p_child*/
		struct list_head *t = &(p->p_child);
		/*una volta trovato il list head,
		vado a cercare chi è il prossimo elemento di p_child,
		 ovvero il primo figlio dopo la sentinella,
		 questo lo faccio grazie al conteiner of*/
		pos = container_of(list_next(t),struct pcb_t,p_sib);
		list_del(list_next(t));
		pos->p_parent=NULL;
		return pos;
	}

	return NULL;
}

pcb_t *outChild(pcb_t *p){
	if(p->p_parent != NULL){
		struct list_head *tmp = &p->p_sib;
		list_del(tmp);
		p->p_parent = NULL;
		return p;
	}

	return NULL;
}

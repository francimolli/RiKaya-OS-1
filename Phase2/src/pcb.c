#include "pcb.h"


void initPcbs(void){
    INIT_LIST_HEAD(&pcbFree_h);
    for(int i = 0; i < MAXPROC; i++){
		list_add_tail(&(pcbFree_table[i].p_next), &pcbFree_h);
	}
}

void freePcb(pcb_t *p){
	    list_add_tail(&p->p_next, &pcbFree_h);
}


HIDDEN void setDefault (pcb_t* p) {
    p->p_parent = NULL;

    p->p_s.entry_hi = 0;
    p->p_s.cause = 0;
    p->p_s.status = 0;
    p->p_s.pc_epc = 0;

    for(int i = 0;i<29;i++){
        p->p_s.gpr[i] = 0;
    }
    p->p_s.hi = 0;
    p->p_s.lo = 0;

    p->p_semkey = NULL;
    p->priority = p->original_priority = 0;
    p->user_time_old = p->kernel_time_old = 0;
    p->user_time_new = p->kernel_time_new = 0;
    p->wallclock_time = 0;
    p->tutor = 0;
    p->command = 0;

    p->oldSYSBP = NULL; 
    p->newSYSBP = NULL;
    p->oldTLB = NULL;	
    p->newTLB = NULL;
    p->oldPGT =	NULL;
    p->newPGT = NULL;


    INIT_LIST_HEAD(&(p->p_next));
    INIT_LIST_HEAD(&(p->p_child));
    INIT_LIST_HEAD(&(p->p_sib));

}

pcb_t *allocPcb(void){

	if (list_empty(&pcbFree_h)) return NULL;

	pcb_t *punt;

	//punt è l'indirizzo del primo elemento di tipo pcb nella coda dei processi
    //e viene "estratto" tramite la sentinella
	punt = container_of(pcbFree_h.next, pcb_t, p_next);

	//rimuovo il pcb dalla lista dei processi vuoti
	list_del(&(punt->p_next));

    //Inizializzo i campi del pcb al loro valore di default
    setDefault(punt);

	return punt;
}

//inizializza lista dei PCB inizializzando la sentinella.
pcb_t* mkEmptyProcQ(struct list_head* head){

	INIT_LIST_HEAD(head);

	return NULL;
}

//restituisco true se la lista puntata da head è vuota, false altrimenti.
int emptyProcQ(struct list_head* head){
	if(list_empty(head)) return TRUE;
	return FALSE;
}

void insertProcQ(struct list_head *head, pcb_t *p){

	pcb_t *temp;
	struct list_head* pos;
	list_for_each(pos, head){
		temp = container_of(pos, pcb_t, p_next);
		if (p->priority > temp->priority){
			__list_add(&p->p_next,pos->prev,pos);
			return;
		}
	}
	list_add_tail(&p->p_next, head);

}

pcb_t* headProcQ(struct list_head* head){
	if (!list_empty(head)){
		return (container_of(head->next,pcb_t,p_next));
	}

	return NULL;
}

pcb_t* removeProcQ(struct list_head* head){
	pcb_t *temp = headProcQ(head);
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
		temp = container_of(pos, pcb_t, p_next);
		if(temp == p){
			list_del(pos);
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

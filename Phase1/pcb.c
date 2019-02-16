#include <stdio.h>

#include <listx.h>
#include <const.h>
#include <types_rikaya.h>
#include <pcb.h>

//struct list_head { 			QUESTA E' LIST_HEAD........ IO MI DO FUOCO.
//	struct list_head *next, *prev;
//};

pcb_t pcbFree_table[MAX_PROC];

struct list_head pcbFree_h;

void initPcbs(){
	INIT_LIST_HEAD(&pcbFree_h); //inizializza i campi di una struttura già esistente
	for(int i = 0; i<MAX_PROC; i++){
		list_add_tail(&(pcbFree_table[j].p_next), &pcbFree_h);
	}
}

void freePcb(pcb_t *p){
	list_add_tail(&p->p_next, &pcbFree_h);
}

pcb_t *allocPcb(){
	if(list_empty(&pcbFree_h)){
		return NULL;
	}else{
		struct pcb_t* punt;
		//punt è l'indirizzo dell'elemento puntato dalla sentinella estratto da list_head.
		punt = container_of(pcbFree_h.next, pcb_t, p_next); 

		punt->p_parent = NULL;
		INIT_LIST_HEAD(&(punt->p_child));
		INIT_LIST_HEAD(&(punt->p_sib));
		INIT_LIST_HEAD(&(punt->p_next));
		//IMPORTANTE, non ho trovato i campi del processor state (p_s) . 
		list_del(pcbFree_h.next);
		return punt;
	}
}

//inizializza lista dei PCB inizializzando la sentinella.
pcb_t* mkEmptyProcQ(struct list_head* head){
	INIT_LIST_HEAD(head); 
	return head;
}

//restituisco true se la lista puntata da head è vuota, false altrimenti.
int emptyProcQ(struct list_head* head){
	if(list_empty(head) return 1;
	else return 0; 
}

void insertProcQ(struct list_head* head, pcb_t* p){
	if(!(list_empty(head))){ 
		int prior = p->priority;
		pcb_t *temp;
		list_head* pos;
		list_for_each(pos, head){
			temp = container_of(head, pcb_t, p_next);
			if (temp->priority < prior){ //se la prorità successiva è minore
				__list_add(&p->next, head->prev, head); //inserisco tra prev e next.
				return;}
			}	
	}
	list_add_tail(&p->p_next, head) //se la lista è vuota.
}

pcb_t headProcQ(struct list_head* head){
	if (list_empty(head)){
		return NULL;
	}else{	
		return containerof(head->next,pcb_t,p_next);	
	}
}

pcb_t* removeProcQ(struct list_head* head){
	pcb_t *temp;
	temp = contanier_of(head, pcb_t, p_next);
	if(temp == NULL) return NULL;
	else{
		list_del(head.next);
		return temp->p_next;
	}
}

pcb_t* outProcQ(struct list_head* head, pcb_t *p){
	list_head *pos;
	list_for_each(pos, head){
		temp = container_of(head, pcb_t, p_next);
		if(temp->p_next == p->p_next){
			list_del(head.next);
			return;
		}
	}
	return NULL;
}

































































	

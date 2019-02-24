#include "asl.h"
#include "pcb.h"
#include <listx.h>
#include <const.h>
#include <stdio.h>

semd_t semd_table[MAXPROC];

semd_t semdFree;
struct list_head semdFree_h;

semd_t ASL;
struct list_head semd_h;

/*getSemd ritorna un puntatore al nodo della ASL con chiave pari a key;
 * se non c'è nessun semaforo attivo nella ASL con tale chiave 
 * la funzione ritorna NULL.*/
semd_t* getSemd(int *key){
	
	if(!list_empty(&ASL.s_next) && key != 0){
		
		struct list_head *pos;
		
		list_for_each(pos, &semd_h){
			
			semd_t* s = container_of(pos, semd_t, s_next);
			
			if(*(s->s_key) == *(key)){
				return s;
			}
		}
	}
	
	return NULL;
}

/*Inserisce un nuovo pcb in coda al semaforo identificato da key. Se tale 
 * semaforo è già presente nella ASL allora basta aggiungere il pcb in coda alla 
 * lista s_procQ del semaforo. Se, invece, il semaforo non è presente è necessario
 * crearne uno di nuovo. Prima di aggiungere un nuovo semd bisogna controllare che
 * la lista dei semd liberi non sia vuota (se ciò accade la funzione ritorna TRUE).
 * Nel caso in cui il semd sia già presente nella ASL o sia possibile aggiungerlo
 * a quest'ultima, la funzione ritorna FALSE. In tutti gli altri possibili casi di
 * default ritorna FALSE.*/
int insertBlocked(int *key, pcb_t *p){
	
	semd_t* s = getSemd(key); 
	
	if(s != NULL){
		insertProcQ(&s->s_procQ, p);
		p->p_semkey = key;
		return FALSE;
	}

	if(list_empty(&semdFree_h)){
		return TRUE;
	}

	s = container_of(semdFree_h.next, semd_t, s_next);
	list_del(semdFree_h.next);
	s->s_key = key;
	INIT_LIST_HEAD(&s->s_next);
	INIT_LIST_HEAD(&s->s_procQ);
	insertProcQ(&s->s_procQ, p);
	struct list_head * pos;
	list_for_each(pos, &semd_h){
		semd_t *temp = container_of(pos, semd_t, s_next);
		if(temp->s_key < s->s_key){
			p->p_semkey = key;
			list_add(&s->s_next, &temp->s_next);
			return FALSE;
		}
	}
	p->p_semkey = key;
	list_add(&s->s_next, &semd_h);
	return FALSE;
}

/*removeBlocked prende in input la chiave identificava di un semd, da cui
 * andare a rimuovere un pcb. Se l'idetificativo è valido, ovvero si trova 
 * nella ASL, allora viene fatta remove dalla testa della lista dei processi
 * bloccati sul quel semaforo. Se il semd non è presente nella ASL, allora 
 * la funzione ritorna NULL, mentre se dopo la rimozione del pcb la coda dei
 * processi del semd è vuota, il semd appartenente all'ASL viene restituito
 * alla semdFree.*/
pcb_t* removeBlocked(int *key){
	//ricerca semd nella ASL
	semd_t* s = getSemd(key);
	
	if(s == NULL){
		return NULL;
	}
	
	if(!list_empty(&s->s_procQ)){
		//semd presente nella ASL, rimozione del primo pcb nella 
		//lista s_procQ
		pcb_t *t = container_of(s->s_procQ.next, pcb_t, p_next);
		t->p_semkey = NULL;
		list_del(s->s_procQ.next);
	
		//se il semd ha s_procQ vuota, allora ritorna il semd a 
		//semdFree
		if(emptyProcQ(&s->s_procQ)){
			
			list_del(&s->s_next);
			INIT_LIST_HEAD(&s->s_next);
			s->s_key = 0;
			INIT_LIST_HEAD(&s->s_procQ);
			
			list_add(&s->s_next, &semdFree_h);
		}

		return t;
	}

	return NULL;
}

/*la funzione prende in input un pcb da rimuovere da un semaforo. Si compie una
 * ricerca nella ASL utilizzando il campo p_semKey del pcb: se avviene un match,
 * allora si ricerca il pcb dentro la lista s_procQ dei processi bloccati su 
 * tale semaforo. Se avviene un match, viene restituito il pcb e quest'ultimo
 * è rimosso dalla coda. In tutti gli altri casi si ritorna NULL.*/
pcb_t* outBlocked(pcb_t *p){
	//ricerca nella ASL
	semd_t* s = getSemd(p->p_semkey);

	if(s != NULL){
		//semd trovato
		if(!list_empty(&s->s_procQ)){
			
			struct list_head *pos;
			
			//ricerca nella coda dei processi bloccati su s
			list_for_each(pos, &s->s_procQ){
				pcb_t* q = container_of(pos, pcb_t, p_next);
				if(p == q){
					list_del(&q->p_next);
					if(list_empty(&s->s_procQ)){
					
						list_del(&s->s_next);

					        INIT_LIST_HEAD(&s->s_next);
						s->s_key = 0;
						INIT_LIST_HEAD(&s->s_procQ);
						
						list_add(&s->s_next, &semdFree_h);
					}

					return q;
				}
			}
		}
	}

	return NULL;
}

/*la funzione prende in input una chiave da ricercare all' interno della ASL:
 * se il semd è attivo allora la funzione ritorna il pcb associato alla testa
 * della lista s_procQ. In tutti gli altri casi la funzione ritorna NULL.*/
pcb_t* headBlocked(int *key){
	//ricerca nella ASL
	semd_t* s = getSemd(key);

	if(s != NULL){
		//controllo che la lista dei processi bloccati su s non sia vuota
		if(!list_empty(&s->s_procQ)) 
			return container_of(s->s_procQ.next, pcb_t, p_next);
	}

	return NULL;
}

/*outChildBlocked rimuove il pcb dal semd indicato in p_semKey. Inoltre la funzione, ricorsivamente,
 * elimina tutti i pcb che hanno come antenato p. Per fare ciò si fa una previsita dell'albero
 * radicato in p, e mano a mano che si scende si scollega il pcb dal proprio semd e, una volta che
 * le chiamate ricorsive iniziano a chiudersi, i vari pcb vengono scollagati da fratelli e dai 
 * propri figli.*/
void outChildBlocked(pcb_t *p){
	//ricerca nella ASL
	semd_t* s = getSemd(p->p_semkey);

	if(s != NULL){
		//semd attivo
		if(!list_empty(&s->s_procQ)){
			//coda dei processi in attesa su s non vuota
			struct list_head *pos;
			
			list_for_each(pos, &s->s_procQ){
				
				pcb_t* q = container_of(pos, pcb_t, p_next);
			
				if(p == q){
					/*se p è in coda allora lo si scollega dalla lista
					*dei processi in coda nel semd s*/		
					list_del(&p->p_next);
					
					/*se il semd ha la coda dei processi vuoti viene restituito
					*a semdFree*/
					if(list_empty(&s->s_procQ)){
					
						list_del(&s->s_next);
						
						INIT_LIST_HEAD(&s->s_next);
						s->s_key = 0;
						INIT_LIST_HEAD(&s->s_procQ);

						list_add(&s->s_next, &semdFree_h);
					}
					
					//puntatore al figlio di p
					pcb_t* c = container_of(p->p_child.next, pcb_t, p_child);
					
					struct list_head *it;
					
					list_for_each(it, &c->p_sib){
						//chiamata ricorsiva sul figlio
						outChildBlocked(c);
						/*una volta arrivati ad una foglia, si esamina il
						*fratello della foglia, se ce ne sono*/
						c = container_of(it, pcb_t, p_sib);
					}
				}
			}
		}
	}	
}

/*funzione per inizializare la tabella dei semd, la lista dei semd liberi e la ASL.*/
void initASL(){
	
	//inizializzazione semd_table
	for(int i = 0; i < MAXPROC; i++){
		INIT_LIST_HEAD(&semd_table[i].s_next);
		semd_table[i].s_key = 0;
		INIT_LIST_HEAD(&semd_table[i].s_procQ);
	}
	
	//inizializzazione semdFree
	INIT_LIST_HEAD(&semdFree.s_next);
	semdFree.s_key = 0;
	INIT_LIST_HEAD(&semdFree.s_procQ);

	//sentinella
	INIT_LIST_HEAD(&semdFree_h);
	
	//aggiunta semd liberi alla semdFree
	for(int i = 0; i < MAXPROC; i++){
		semd_t* s = &semd_table[i];
		list_add(&s->s_next, &semdFree_h);
	}
	
	//inizializzazione ASL
	INIT_LIST_HEAD(&ASL.s_next);
	ASL.s_key = 0;
	INIT_LIST_HEAD(&ASL.s_procQ);
	
	//sentinella
	INIT_LIST_HEAD(&semd_h);
}

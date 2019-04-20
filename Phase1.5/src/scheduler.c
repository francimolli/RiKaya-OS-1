#include "myconst.h"
#include "scheduler.h"
#include <umps/libumps.h>

extern void addokbuf(char *strp);
extern void log_process_order(int process);
/*
 * In questa fase, il sistema operativo entra nell'entry point (il main)
 * dove vengono inizializzate le varie strutture e dopodichè passa il testimone
 * allo scheduler che si occupa totalmente da quel momento in poi della gestione
 * dei processi. Per questa fase ipotizziamo che venga, fuori dallo scheduler (nel main)
 * inizializzata una coda di processi pronti (test1, test2, test3) che passiamo come parametro
 * allo scheduler, il quale si occuperà della loro corretta esecuzione.
 * Idealmente, lo scheduler dovrebbe permettere l'aggiunta a runtime di ulteriori processi
 * nella coda dei processi pronti, ma questo richiederebbe un (INTERRUPT o SYSCALL?) da parte
 * del sistema operativo ad informare lo scheduler della presenza di un nuovo processo.
 * Per questa fase ci limitiamo a gestire i 3 processi di test.
*/


//readyQueue_h = sentinella gestita dallo scheduler relativa alla coda dei processi pronti

void  context() {

		//controlliamo che il processore sia libero
		if(curr_proc != NULL){
			/*se il proccessore non è libero,ovvero curr_proc non sta puntando ad un pcb,
			si riporta la priorità del processo corrente a original_priority*/
			curr_proc->priority = curr_proc->original_priority;

			/*prima di inserire il processo di nuovo nella ready_queue per fare il context switch,
			si incrementano la priorità di tutti i processi nella ready_queue per evitare la starvation*/
			struct list_head* iter ;
			list_for_each(iter,&ready_queue_h){
				container_of(iter,pcb_t,p_next)->priority++;
			}
			
			//reinserimento nella coda dei processi pronti ad essere eseguiti
			insertProcQ(&ready_queue_h, curr_proc);
		}
		//processo con priorità più alta viene tolto dalla ready_queue per essere eseguito
		curr_proc = removeProcQ(&ready_queue_h);
		
		//inserimento della priorità originale del processo in esecuzione nel diagramma di gantt
		log_process_order(curr_proc->original_priority);

		//carica lo stato del processo
		LDST(&curr_proc->p_s);
}

void  scheduler(){
	//controllo se ci sono processi da eseguire
	if(emptyProcQ(&ready_queue_h)){
		HALT();
	}

	context();

}


void kill_proc(){
		rec_proc_kill(curr_proc);
		curr_proc=NULL;
		scheduler();
}

pcb_t* rec_proc_kill(pcb_t* tmp){
	if (tmp==NULL){
		return NULL;
	}
	else{
		return rec_proc_kill(outProcQ(&ready_queue_h,outChild(tmp)));
	}
}

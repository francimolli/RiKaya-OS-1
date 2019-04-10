#include "myconst.h"
#include "scheduler.h"
#include <umps/libumps.h>

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

void scheduler () {

	//controllo se ci sono processi da eseguire

	while(1){
		if(emptyProcQ(&ready_queue_h)){
			HALT();
		}

		//controlliamo che il processore sia libero
		if(curr_proc != NULL){
			//se il proccessore non è libero si riporta la priorità del processo corrente
			//a original_priority
			curr_proc->priority = curr_proc->original_priority;

			insertProcQ(&ready_queue_h, curr_proc);


		}
		curr_proc = removeProcQ(&ready_queue_h);
		log_process_order(curr_proc->original_priority);

		struct list_head* iter ;
		list_for_each(iter,&ready_queue_h){
			if(container_of(iter,pcb_t,p_next)->priority < curr_proc->priority){
				container_of(iter,pcb_t,p_next)->priority++;}
		}

		//carica lo stato del processo
		LDST(&curr_proc->p_s);
	}
}

void context(){
	
}


void kill_proc(){
		pcb_t* tmp=rec_proc_kill(&curr_proc);
		curr_proc=NULL;
}
pcb_t* rec_proc_kill(pcb_t* tmp){
	if (tmp==NULL){
		return NULL;
	}else{
		return rec_proc_kill(outProcQ(&ready_queue_h,outChild(tmp)));
	}
}

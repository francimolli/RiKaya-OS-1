#include "scheduler.h"

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

void  context() {

		//controlliamo che il processore sia libero
		if(curr_proc != NULL){

			/*se il proccessore non è libero, ovvero curr_proc sta puntando ad un pcb,
			si riporta la priorità del processo corrente a original_priority*/
			curr_proc->priority = curr_proc->original_priority;

			/*prima di inserire il processo di nuovo nella ready_queue per fare il context switch,
			si incrementano la priorità di tutti i processi nella ready_queue per evitare la starvation*/
			struct list_head* iter ;
			list_for_each(iter,&ready_queue_h){
				container_of(iter,pcb_t, p_next)->priority++;
			}

			//gestione tempo esecuzione del processo, si passa da kernel mode a user mode
			if(curr_proc->kernel_time_old >= 0){
				curr_proc->kernel_time_old += getTODLO() - curr_proc->kernel_time_new;
				curr_proc->kernel_time_new = 0;
			}

			//reinserimento nella coda dei processi pronti ad essere eseguiti
			insertProcQ(&ready_queue_h, curr_proc);
		}
		//processo con priorità più alta viene tolto dalla ready_queue per essere eseguito
		curr_proc = removeProcQ(&ready_queue_h);

		//tracciamento tempo esecuzione processo
		if(!curr_proc->wallclock_time)
			curr_proc->wallclock_time = getTODLO();

		curr_proc->user_time_new = getTODLO();

		//carica lo stato del processo
		LDST(&curr_proc->p_s);
}

void  scheduler(){

	//controllo se ci sono processi da eseguire
	if(emptyProcQ(&ready_queue_h)){
		if(curr_proc != NULL)
			context();
		else{
			if(ProcBlocked > 0){
				setTIMER(TIME_SLICE);
				setSTATUS((getSTATUS() | STATUS_IEc) | STATUS_INT_UNMASKED);
				WAIT();
			}
			else
				HALT();
		}
	}
	else
		context();
}

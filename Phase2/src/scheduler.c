#include "scheduler.h"

/*
 * In questa fase, il sistema operativo entra nell'entry point (il main)
 * dove vengono inizializzate le varie strutture e dopodichè passa il testimone
 * allo scheduler che si occupa totalmente da quel momento in poi della gestione
 * dei processi. Per questa fase ipotizziamo che venga, fuori dallo scheduler (nel main)
 * inizializzato il PCB del processo test che passiamo come parametro
 * allo scheduler, inserendolo nella ready_queue.
 * Nello scheduler possono essere aggiunti a run-time nuovi processi creati con la syscall CREATEPROCESS,
 * come è possibile che il processo corrente o quelli nella ready_queue vengano eliminati dalla syscall
 * TERMINATEPROCESS.
 * Questo scheduler, prima di fare il context switch e fare l'aging, controlla se la ready_queue è vuota:
 * se quest'ultima è vuota e anche il processo corrente è NULL, non significa che il sistema non abbia più
 * possibili processi da sistemare, infatti potrebbero essere presenti processi bloccati su semafori. Quando
 * ciò accade si usa la WAIT per attendere un interrupt dal processor local timer, o dall'interval timer, o
 * da qualche dispositvo di I/O.
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
			
			//reinserimento nella coda dei processi pronti ad essere eseguiti
			insertProcQ(&ready_queue_h, curr_proc);
		}
		//processo con priorità più alta viene tolto dalla ready_queue per essere eseguito
		curr_proc = removeProcQ(&ready_queue_h);

		/*time management: se è la prima attivazione del processo, ovvero wallclock time pari a 0, allora
		si assegna il valore ritornato dalla funzione getTODLO() a wallclock_time. In più il processo si trova in
		user mode appena caricato perchè esegue il suo codice e non quello di un'eccezione.*/
		if(!curr_proc->wallclock_time)
			curr_proc->wallclock_time = getTODLO();

		curr_proc->user_time_new = getTODLO();

		//carica lo stato del processo
		LDST(&curr_proc->p_s);
}

void  scheduler(){

	//controllo se ci sono processi da eseguire
	if(emptyProcQ(&ready_queue_h)){
		//ready_queue vuota, controllo se il processo corrente è NULL
		if(curr_proc != NULL)
			context();
		else{
			//processo corrente NULL, si controlla se il numero di processi bloccati è 0
			if(ProcBlocked > 0){
				setTIMER(TIME_SLICE);//interrupt nel caso in cui nel frattempo la ready_queue sia divenuta non vuota
				setSTATUS((getSTATUS() | STATUS_IEc) | STATUS_INT_UNMASKED);//interrupt abilitati
				WAIT();
			}
			else
				HALT();
		}
	}
	else
		context();
}

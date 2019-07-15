#include "syscalls.h"

void Get_CPU_Time(U32 *user, U32 *kernel, U32 *wallclock){
	/*aggiorno valori del tempo d'esecuzione (lo user_time non necessita di essere aggiornato
	in quanto non è cambiato durante l'esecuzione della syscall)*/
	curr_proc->p_s.kernel_time_old = getTODLO() - curr_proc->p_s.kernel_time_new;

	*user = curr_proc->p_s.user_time_old;
	*kernel = curr_proc->p_s.kernel_time_old;
	*wallclock = getTODLO() - curr_proc->p_s.wallclock_time;
}

int Create_Process(state_t *statep, int priority, void **cpid){

		//alloco un nuovo pcb_t
		pcb_t *child_proc = allocPcb();

		//se la lista dei pcb liberi (pcbFree) è vuota allora non è stato possibile allocare il pcb
		if(child_proc == NULL)
			return -1;

		//pcb allocato
		cpy_state(statep, &child_proc->p_s);//set campo p_s del nuovo pcb
		child_proc->original_priority = child_proc->priority = priority;//set priorità del nuovo pcb

		//child_proc viene reso figlio del chiamante ovvero curr_proc
		insertChild(curr_proc, child_proc);

		//inserimento nella coda dei processi del nuovo pcb
		insertProcQ(&ready_queue_h, child_proc);

		/*nota: se si considera come prima attivazione del processo l'inserimento nella ready_queue allora
		bisognerebbe fare time management, ma secondo me la prima attivazione avviene quando lo scheduler
		sceglie per la prima volta il processo*/

		//a questo punto la chiamata ha successo, se cpid != NULL dunque cpid contiene l'indirizzo di child_proc
		if(cpid != NULL)
			*((pcb_t *) cpid) = child_proc;



		return 0;
}

int Terminate_Process(void **pid){

		/*quando un processo viene terminato:
			-il processo da eliminare va tolto dalla ready_queue_h
			-la lista dei processi liberi (pcbFree) riceve un nuovo pcb_t allocabile
			-va tolto dalle code dei semafori in cui il processo potrebbe essere bloccato
		Inoltre se il processo terminato è il curr_proc, allora va tolto dal processore il processo corrente
		e richiamato lo scheduler*/

		//NB: probabilmente si può fare meglio in termini di costo computazionale

		//caso in cui pid sia il curr_proc
		if(pid == NULL || pid == 0){

			//curr_proc non ha padre, errore
			if(curr_proc->p_parent == NULL)
				return -1;

			//curr_proc ha padre, si cerca il tutor per la progenie di curr_proc (la radice è sempre tutor)
			pcb_t *tmp = curr_proc->p_parent;
			while(tmp->tutor == 0)
				tmp = tmp->p_parent;

			//tmp è il tutor, inserisco come figli di tmp i figli di curr_proc, se curr_proc ha figli
			if(!emptyChild(curr_proc)){
				struct list_head* iter ;
				list_for_each(iter,&ready_queue_h){
					container_of(iter,pcb_t,p_next)->p_parent = tmp;
				}
			}

			//si toglie il curr_proc dalla ready_queue_h
			if(outProcQ(&ready_queue_h, curr_proc) == NULL)
				return -1;

			//si toglie il curr_proc dalla lista dei figli del padre (che c'è di sicuro)
			outChild(curr_proc->p_parent);

			//si restituisce il pcb alla pcbFree list
			freePcb(curr_proc);

			//si toglie il pcb del semaforo in cui è bloccato
			if(outBlocked(curr_proc) == NULL)
				return -1;

			scheduler();

			return 0;
		}

		/*caso in cui pid punta ad un processo che non è quello corrente:
		il processo da terminare deve essere discendente del processo corrente*/

		pcb_t *q = *((pcb_t *)pid);
		if(lookup_proc(curr_proc, q)){

			pcb_t *tmp = q->p_parent;
			while(tmp->tutor == 0)
				tmp = tmp->p_parent;

			if(!emptyChild(q)){
				struct list_head* iter ;
				list_for_each(iter,&ready_queue_h){
					container_of(iter,pcb_t,p_next)->p_parent = tmp;
				}
			}

			if(outProcQ(&ready_queue_h, q) == NULL)
				return -1;

			outChild(q->p_parent);
			freePcb(q);
			if(outBlocked(q) == NULL)
				return -1;

			return 0;
		}

		return -1;
}

int lookup_proc(pcb_t *p, pcb_t *q){
	if(q->p_parent == p)
		return 1;
	else if(q->p_parent == NULL)
		return 0;
	else return lookup_proc(p, q->p_parent);
}

void Verhogen(int *semaddr){

	/*operazione di rilascio sul semaforo indicato da semaddr:
	se il descrittore del semaforo è presente nella ASL allora il processo che viene rimosso
	dalla coda dei processi bloccati deve essere risvegliato e messo nella ready_queue_h*/
	pcb_t *p = removeBlocked(semaddr);
	*semaddr++;

	if(*semaddr <= 0){
		if(p != NULL){
			p->p_semkey = NULL;
			insertProcQ(&ready_queue_h, p);
		}
	}
}

void Passeren(int *semaddr){

	/*operazione di richiesta del semaforo indicato da semaddr:
	se il descrittore del semaforo è presente nella ASL allora il processo che viene bloccato
	su quel semaforo, il processo corrente, viene tolto dal processore per
	essere inserito nella lista di processi bloccati sul semaforo indicato da semaddr*/
	*semaddr--;

	if(*semaddr < 0){
		if(insertBlocked(semaddr, curr_proc) == FALSE){
			curr_proc->p_semkey = semaddr;
			outProcQ(&ready_queue_h, curr_proc);
			curr_proc = NULL;
			scheduler();
		}
	}
}

void Wait_Clock(){

	/*sospende il processo che la invoca fino al prossimo tick. Dunque l'operazione richiesta è quella
	di una Passeren. Risulta necessario creare un nuovo descrittore(grazie alla funzione insertBlocked
	definita nella fase 1 per la ASL). Una volta passato il tick del clock del sistema sarà necessario
	fare un numero di Verhogen in modo da risvegliare tutti i processi bloccati.*/

	if(semDevices[CLOCK_SEM] == 0)//siginifica che il semaforo è vuoto, ovvero nessuno ha richiesto la Wait_Clock
		SET_IT(100);
	Passeren(&semDevices[CLOCK_SEM], curr_proc);

}

int Do_IO(U32 command, U32 *register, U32 term_command){

	curr_proc->command = command;
	curr_proc->recv_or_transm = term_command;
	woken_proc = curr_proc;
	IO_request(command, register, term_command);
	return *dev_status->status;
}

void Set_Tutor(){

	//il curr_proc può diventare padre dei processi orfani
	curr_proc->tutor = 1;
}

int Spec_Passup(int type, state_t *old, state_t *new){

	/*questa system call specifica, all'interno di un campo state_t * del pcb, quale Handler
	di livello superiore chiamare in caso di trap. Questa syscall può essere chiamata una sola volta,
	perciò se un puntatore tra oldSYSBP/newSYSBP, oldTLB/newTLB, oldPGT/newPGT è diverso da NULL, la
	syscall fallisce.*/

	if(curr_proc->oldSYSBP != NULL ||
		 curr_proc->newSYSBP != NULL ||
		 curr_proc->oldTLB != NULL ||
	 	 curr_proc->newTLB != NULL ||
	 	 curr_proc->oldPGT != NULL ||
	 	 curr_proc->newPGT != NULL)
		 return -1;

	switch(type){
		case 0:

			//caso syscall/breakpoint
			curr_proc->oldSYSBP = old;
			curr_proc->newSYSBP = new;
			return 0;

			break;

		case 1:

			//caso TLB
			curr_proc->oldTLB = old;
			curr_proc->newTLB = new;
			return 0;

			break;

		case 2:

			//caso Program Trap
			curr_proc->oldPGT = old;
			curr_proc->newPGT = new;
			return 0;

			break;

		default:

			return -1;

			break;
	}
}

void Get_pid_ppid(void **pid, void **ppid){

	/*assegna l'identificativo del curr_proc a *pid (se pid != NULL),
	e l'identificativo del processo genitore a *ppid (se ppid != NULL)*/
	if(pid != NULL)
		*((pcb_t *) pid) = curr_proc;

	if(ppid != NULL)
		*((pcb_t *) ppid) = curr_proc->p_parent;
}

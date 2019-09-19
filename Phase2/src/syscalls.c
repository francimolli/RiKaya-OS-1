#include "syscalls.h"
extern void addokbuf(char *strp);
void Get_CPU_Time(U32 *user, U32 *kernel, U32 *wallclock){
	/*aggiorno valori del tempo d'esecuzione: lo user_time non necessita di essere aggiornato
	in quanto non è cambiato durante l'esecuzione della syscall, a differenza del kernel_time
	che è iniziato quando si è entrati nel case della syscall(alla fine dello switch delle syscall
	il kernel sarà ulteriormente aggiornato, perciò basta solo leggere il TODLOW prima di restituire
	il valore del kernel time)*/
	curr_proc->kernel_time_old += getTODLO() - curr_proc->kernel_time_new;

	if(user != NULL && user != 0)
		*user = curr_proc->user_time_old;
	if(kernel != NULL && kernel != 0)
		*kernel = curr_proc->kernel_time_old;
	if(wallclock != NULL && wallclock != 0)
		*wallclock = getTODLO() - curr_proc->wallclock_time;
}

int Create_Process(state_t *statep, int priority, void **cpid){

		//alloco un nuovo pcb_t
		pcb_t *child_proc = allocPcb();

		//se la lista dei pcb liberi (pcbFree) è vuota allora non è stato possibile allocare il pcb
		if(child_proc == NULL)
			return -1;
		else{
			//pcb allocato
			cpy_state(statep, &child_proc->p_s);//set campo p_s del nuovo pcb
			child_proc->original_priority = priority;//set priorità del nuovo pcb
			child_proc->priority = priority;

			//child_proc viene reso figlio del chiamante ovvero curr_proc
			insertChild(curr_proc, child_proc);

			//inserimento nella coda dei processi del nuovo pcb
			insertProcQ(&ready_queue_h, child_proc);

			//a questo punto la chiamata ha successo, se cpid != NULL allora cpid contiene l'indirizzo di child_proc
			if(cpid != NULL && cpid != 0){
				*((pcb_t **)cpid) = child_proc;
			}

			return 0;
		}
}

int Terminate_Process(void **pid){

		/*quando un processo viene terminato:
			-il processo da eliminare va tolto dalla ready_queue_h
			-la lista dei processi liberi (pcbFree) riceve un nuovo pcb_t allocabile(si usa freePcb)
			-va tolto dalle code dei semafori in cui il processo potrebbe essere bloccato(se non è il curr_proc)
		Inoltre se il processo terminato è il curr_proc, allora va tolto dal processore il processo corrente
		e richiamato lo scheduler*/
		pcb_t *target_proc;

		if(pid == NULL || pid == 0){
			//il processo da eliminare è quello corrente
			target_proc = curr_proc;
		}
		else{
			//il processo da eliminare non è quello corrente
			target_proc = *((pcb_t **)pid);
		}

		//se il processo da eliminare non ha padre, vuol dire che è il root processo, errore
		if(target_proc->p_parent == NULL)
			return -1;

		/*se pid != NULL allora esso deve trovarsi nella progenie del curr_proc,
		dato che in C le espressioni sono valutate in modo lazy è possibile scrivere la seguente condizione.
		lookup_proc ritorna false se il processo non si trova nella progenie*/
		if(pid != NULL && pid != 0 && !lookup_proc(curr_proc, target_proc))
			return -1;

		//la progenie del processo da terminare non viene eliminata
		if(!emptyChild(target_proc)){
			//il processo ha figli, vanno inseriti come figli del primo processo indicato come tutor (alla peggio è il root)
			pcb_t *tutor = target_proc->p_parent;
			while(!tutor->tutor)
				tutor = tutor->p_parent;

			//offspring inserita come child del tutor
			pcb_t *child;
			do{
				child = removeChild(target_proc);
				insertChild(tutor, child);
			}while(child != NULL);
		}

		//il target_proc va eliminato dalla lista dei figli del padre
		outChild(target_proc);

		if(target_proc->p_semkey != NULL){
			//il processo da eliminare non è quello corrente perchè è bloccato su un semaforo (la sua p_semkey è != da NULL)
			int *sem = target_proc->p_semkey;
			outBlocked(target_proc);
			//incremento si fa su sem, dopo outBlocked non si può più accedere alla key del semaforo tramite target_proc
			(*sem)++;
			ProcBlocked--;

			//il processo va tolto anche dalla ready_queue se non è quello corrente
			outProcQ(&ready_queue_h, target_proc);
		}

		freePcb(target_proc);

		if(curr_proc == target_proc){
			curr_proc = NULL;
			scheduler();
		}

		return 0;
}

//ritorna true se il processo da terminare si trova all'interno della progenie del curr_proc
int lookup_proc(pcb_t *curr, pcb_t *q){
	if(q->p_parent == curr)
		return TRUE;
	else if(q->p_parent == NULL)
		return FALSE;
	else return lookup_proc(curr, q->p_parent);
}

void Verhogen(int *semaddr){

	/*operazione di rilascio sul semaforo indicato da semaddr:
	se il descrittore del semaforo è presente nella ASL allora il processo che viene rimosso
	dalla coda dei processi bloccati deve essere risvegliato e messo nella ready_queue_h*/
	pcb_t *p = removeBlocked(semaddr);
	(*semaddr)++;

	if(*semaddr <= 0){
		if(p != NULL){

			p->p_semkey = NULL;
			ProcBlocked--;
			insertProcQ(&ready_queue_h, p);
		}
	}
}

void Passeren(int *semaddr){

	/*operazione di richiesta del semaforo indicato da semaddr:
	se il descrittore del semaforo è presente nella ASL allora il processo che viene bloccato
	su quel semaforo, il processo corrente, viene tolto dal processore per
	essere inserito nella lista di processi bloccati sul semaforo indicato da semaddr*/
	(*semaddr)--;

	if(*semaddr < 0){
		if(!insertBlocked(semaddr, curr_proc)){
			ProcBlocked++;
			curr_proc->p_semkey = semaddr;

			/*time management: il processo verrà tolto dalla ready queue, perciò è necessario aggiornare qui
			il suo kernel time(altrimenti verrebbe fatto dentro il context switch, ma ciò avviene solo per i processi
			nella ready queue)*/

			if(curr_proc->kernel_time_new > 0){
				curr_proc->kernel_time_old += getTODLO() - curr_proc->kernel_time_new;
				curr_proc->kernel_time_new = 0;
			}

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
	fare un numero di Verhogen pari al numero di processi che hanno invocato la waitclock,
	in modo da risvegliare tutti i processi bloccati.*/

	Passeren(pseudoclock_sem.s_key);

}

void Do_IO(U32 command, U32 *reg, U32 term_command){

	int *semaddr;
  U32 offset = 0;

	/*si tiene nota del comando da dare al device per fare I/O,
	tornerà utile quando il processo verrà risvegliato*/
	curr_proc->command = command;

  /*è necessario ottenere un indice all'interno di semDevices, ovvero ottener il semaforo associato
  al device che richiede di fare I/O. Per fare ciò si considera che da DEV_REGS_START a TERM0ADDR sono
  presenti le locazioni di memoria dedicate ai devices quali disk, tape, network, printer. Se reg è superiore a
  TERM0ADDR, allora il semaforo è associato ad un terminale e il commando può esssere associato alla
  trasmissione o alla ricezione in base al contenuto della variabile term_command.*/

	//usa offset semafori e non registri per discriminare il device
	if(reg >= (U32 *)TERM0ADDR){
    //calcolo offset all'interno dell'array di semafori per il terminale
    offset = (reg - (U32 *)TERM0ADDR)/DEV_REG_SIZE_W;

    //registro associato ad un terminale
    if(term_command){

      //richiesta di I/O su un terminale in ricezione, si scrive il comando nel campo recv_command
      semaddr = semDevices.terminalR[offset].s_key;

			//semaforo con valore 1, terminale libero in ricezione
			if(!*semaddr)
				((termreg_t *)reg)->recv_command = command;
    }
  	else{

      //richiesta di I/O su un terminale in trasmissione, si scrive il comando nel campo transm_command
      semaddr = semDevices.terminalT[offset].s_key;

			//semaforo con valore 1, terminale libero in trasmissione
			if(!*semaddr)
				((termreg_t *)reg)->transm_command = command;
    }
  }
  else{
    /*per calcolare l'offset è necessario sapere a quale indirizzo di memoria iniziano i registri dedicati ad
    ogni diverso tipo di device tra disk, tape, network e printer. Per ottenere l'indirizzo di memoria in cui
    si trovano i registri associati ai disk, tape, network o printer si usa la macro DEV_ADDRESS(LINENO, DEVNO),
    utilizzando come DEVNO lo 0. Per esempio per ottenere l'indirizzo di memoria da cui parte la memorizzazione
    dei devices di tipo tape, si usa DEV_ADDRESS(INT_TAPE - INT_LOWEST, 0). Infine per ottenere l'offset
    relativo al device che ha il registro reg, si procede come per il terminale.*/

    if(reg >= (U32 *)DISK_START && reg < (U32 *)TAPE_START){
      offset = (reg - (U32 *)DISK_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.disk[offset].s_key;
    }
    else if(reg >= (U32 *)TAPE_START && reg < (U32 *)NETWORK_START){
      offset = (reg - (U32 *)TAPE_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.tape[offset].s_key;
    }
    else if(reg >= (U32 *)NETWORK_START && reg < (U32 *)PRINTER_START){
      offset = (reg - (U32 *)NETWORK_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.network[offset].s_key;
    }
    else{
      offset = (reg - (U32 *)PRINTER_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.printer[offset].s_key;
    }

		//si scrive il comando nel campo command se il semaforo ha valore 1
		if(!*semaddr)
			((dtpreg_t *)reg)->command = command;
  }

	//il processo che richiede I/O va bloccato
	if(wakeup_proc == curr_proc){
			Passeren(semaddr);
	}
	else{
		/*se il processo ad aver chiamato la Do_IO è il processo risvegliato dopo un'operazione di I/O
		allora non si richiama lo scheduler dopo aver fatto la P perciò non utilizzo la Passeren*/
		(*semaddr)--;
		ProcBlocked++;
		insertBlocked(semaddr, wakeup_proc);
		outProcQ(&ready_queue_h, wakeup_proc);
	}
}

void Set_Tutor(){

	//il curr_proc può diventare padre dei processi orfani
	curr_proc->tutor = TRUE;
}

int Spec_Passup(int type, state_t *old, state_t *new){

	/*questa system call specifica, all'interno di un campo state_t * del pcb, quale Handler
	di livello superiore chiamare in caso di trap. Questa syscall può essere chiamata una sola volta,
	perciò se un puntatore tra oldSYSBP/newSYSBP, oldTLB/newTLB, oldPGT/newPGT è diverso da NULL, la
	syscall fallisce.*/

	if((curr_proc->oldSYSBP != NULL || curr_proc->newSYSBP != NULL) && type == 0)
		 return -1;

	if((curr_proc->oldTLB != NULL || curr_proc->newTLB != NULL) && type == 1)
		 return -1;

	if((curr_proc->oldPGT != NULL || curr_proc->newPGT != NULL) && type == 2)
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
	if(pid)
		*pid = curr_proc;

	if(ppid)
		*ppid = curr_proc->p_parent;
}

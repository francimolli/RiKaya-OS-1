#include "handler.h"

/*
* Handler è la funzione il cui indirizzo di memoria è caricato nelle old area delle possibili eccezioni.
* Quando si verifica un'eccezione questa procedure entra in funzione e in base al contenuto del campo ExcCode
* presente nel registro cause si discrimina tra le varie eccezioni.
* Se si tenta di accedere a questa funzione in user mode si solleva una progra trap.
* N.B. quando il processore è in WAIT state, allora il processo corrente è NULL, e quindi l'unica possibile
* eccezione che si può verificare è l'interrupt.
*/

void Handler() {

	//creo una variabile che contiene il codice dell'eccezione inizializzata
	int cause_code = 0;

	if(curr_proc != NULL){

		cause_code = CAUSE_EXCCODE_GET(getCAUSE());

		//controllo se l'accesso sia in usermode, altrimenti si passa la gestione al PgmTrapHandler();
		if((curr_proc->p_s.status >> 1) & 0x00000001){

			/*accesso in user mode, per fronteggiare l'eccezione, in questo caso una Reserved Instruction,
			si imposta il campo Cause.ExcCode con il valore EXC_RESERVEDINSTR*/
			curr_proc->p_s.cause = CAUSE_EXCCODE_SET(curr_proc->p_s.cause, EXC_RESERVEDINSTR);

			//si richiama il gestore di livello superiore, il PgmTrapHandler;
			PgmTrapHandler();
		}

		/*time management: per qualsiasi eccezione che si sia sollevata, il tempo passato in user mode.
		Se l'eccezione che si è verificata è un interrupt si ferma solo lo user time, mentre se l'eccezione
		che si verifica è una system call, allora si fa partire anche il tempo passato in kernel mode.*/
		if(curr_proc != NULL){
			if(curr_proc->user_time_new > 0){
				curr_proc->user_time_old += getTODLO() - curr_proc->user_time_new;
				curr_proc->user_time_new = 0;
			}
		}

	}
	else{
		//caso in cui non ci siano processi da schedulare ma ci siano processi bloccati
		cause_code = EXC_INTERRUPT;
	}

	switch(cause_code){

		case EXC_INTERRUPT : //handler INTERRUPT

				/*quando si affronta un interrupt lo si affronta con la kernel mode abilitata,
				la virtual memory disabilitata e gli interrupts disabilitati come già settato
				in fase di inizializzazione*/

				Interrupt_Handler();

				/*time management: quando si esce da un interrupt non si può fare altro che ritornare in user mode
				dato che quando si affronta un interrupt, lo si affronta ad interrupt disabilitati, perciò
				un'altra eccezione non può interrompere un interrupt.*/
				if(curr_proc != NULL)
					curr_proc->user_time_new = getTODLO();

				//il ramo else è per gestire il ritorno da un interrupt sollevato durante lo stato di wait
				if(curr_proc != NULL)
					LDST((state_t*) INT_OLDAREA);
				else
					scheduler();
				break;

		case EXC_TLBMOD :

			TLBtrap();

			break;

		case EXC_TLBINVLOAD :

			TLBtrap();

			break;

		case EXC_TLBINVSTORE :

			TLBtrap();

			break;

		case EXC_ADDRINVLOAD :

			PGMtrap();

			break;

		case EXC_ADDRINVSTORE :

			PGMtrap();

			break;

		case EXC_BUSINVFETCH :

			PGMtrap();

			break;

		case EXC_BUSINVLDSTORE :

			PGMtrap();

			break;

		case EXC_SYSCALL : //handler SYSCALL

				//time management
				curr_proc->kernel_time_new = getTODLO();

				//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
				cpy_state((state_t*) SYSBK_OLDAREA, &curr_proc->p_s);

				//incremento del program counter di una WORD_SIZE
				curr_proc->p_s.pc_epc += WORD_SIZE;

				int result = 0;

				//identificazione tipo di system call, reg_a0
				switch((int) curr_proc->p_s.reg_a0){
					case GETCPUTIME :

						Get_CPU_Time((U32 *) curr_proc->p_s.reg_a1, (U32 *) curr_proc->p_s.reg_a2, (U32 *) curr_proc->p_s.reg_a3);

						break;

					case CREATEPROCESS :

						result = Create_Process((state_t *) curr_proc->p_s.reg_a1,(int) curr_proc->p_s.reg_a2, (void **) curr_proc->p_s.reg_a3);

						break;

					case TERMINATEPROCESS :

						result = Terminate_Process((void **) curr_proc->p_s.reg_a1);

						break;

					case VERHOGEN :

						Verhogen((int *) curr_proc->p_s.reg_a1);

						break;

					case PASSEREN :

						Passeren((int *) curr_proc->p_s.reg_a1);

						break;

					case WAITCLOCK :

						Wait_Clock();

						break;

					case WAITIO :

						/*assegnamento fatto qui in modo che all'interno della Do_IO si capisca che la richiesta di I/O
						arrivi dal processo corrente e non da un processo risvegliato da un'interrupt.*/
						wakeup_proc = curr_proc;
						Do_IO((U32) curr_proc->p_s.reg_a1, (U32 *) curr_proc->p_s.reg_a2, (U32) curr_proc->p_s.reg_a3);
						result = curr_proc->p_s.reg_v0;
						break;

					case SETTUTOR :

						Set_Tutor();

						break;

					case SPECPASSUP :

						result = Spec_Passup((int) curr_proc->p_s.reg_a1, (state_t *) curr_proc->p_s.reg_a2, (state_t *) curr_proc->p_s.reg_a3);

						break;

					case GETPID :

						Get_pid_ppid((void **) curr_proc->p_s.reg_a1, (void **) curr_proc->p_s.reg_a2);

						break;

					default	:

						/*se la syscall chiamata è superiore a 10 si richiama un gestore di trap di tipo sysbp se presente,
						altrimenti il processo corrente viene terminato e lo scheduler viene richiamato*/
						if(curr_proc->newSYSBP != NULL)
							SysBpTrapHandler();
						else{
							Terminate_Process(0);
							curr_proc = NULL;
							scheduler();
						}
						break;
				}

				curr_proc->p_s.reg_v0 = (U32) result;

				//time management
				if(curr_proc->kernel_time_new > 0){
					curr_proc->kernel_time_old += getTODLO() - curr_proc->kernel_time_new;
					curr_proc->kernel_time_new = 0;
				}
				curr_proc->user_time_new = getTODLO();

				LDST(&curr_proc->p_s);
			break;

		case EXC_BREAKPOINT : //handler BREAKPOINT

			SYSBPtrap();

			break;

		case EXC_RESERVEDINSTR :

			PGMtrap();

			break;

		case EXC_COPROCUNUSABLE :

			PGMtrap();

			break;

		case EXC_ARITHOVERFLOW :

			PGMtrap();

			break;

		case EXC_BADPTE :

			TLBtrap();

			break;

		case EXC_PTEMISS :

			TLBtrap();

			break;

		default :

			/*l'eccezione sollevata non corrisponde a nessun valore possibile in Cause.ExcCode:
			il processo viene terminato*/

	    Terminate_Process(0);

	    curr_proc = NULL;

	    scheduler();

		break;
	}
}

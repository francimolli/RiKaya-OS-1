#include "handler.h"
extern void addokbuf(char *strp);
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
	}
	else{

		//caso in cui non ci siano processi da schedulare ma ci siano processi bloccati
		cause_code = EXC_INTERRUPT;
	}

	switch(cause_code){

		case EXC_INTERRUPT : //handler INTERRUPT

				/*quando si affronta un interrupt lo si affronta con la kernel mode abilitata,
				la virtual memory disabilitata, come già settato in fase di inizializzazione*/

				//time management
				if(curr_proc != NULL){
					if(curr_proc->user_time_new > 0){
						curr_proc->user_time_old += getTODLO() - curr_proc->user_time_new;
						curr_proc->user_time_new = 0;
					}
				}

				Interrupt_Handler();

				//time management
				if(curr_proc != NULL){
					curr_proc->user_time_new = getTODLO();
				}

				if(curr_proc)
					LDST((state_t*) INT_OLDAREA);
				else
					scheduler();
				break;

		case EXC_TLBMOD :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) TLB_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			TlbTrapHandler();

			break;

		case EXC_TLBINVLOAD :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) TLB_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			TlbTrapHandler();

			break;

		case EXC_TLBINVSTORE :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) TLB_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			TlbTrapHandler();

			break;

		case EXC_ADDRINVLOAD :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_ADDRINVSTORE :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_BUSINVFETCH :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_BUSINVLDSTORE :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_SYSCALL : //handler SYSCALL

				if(curr_proc->user_time_new > 0){
					curr_proc->user_time_old += getTODLO() - curr_proc->user_time_new;
					curr_proc->user_time_new = 0;
				}
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

						wakeup_proc = curr_proc;/*assegnamento fatto qui in modo che all'interno della Do_IO
																			si capisca che la richiesta di I/O arrivi dal processo corrente
																			e non da un processo risvegliato da un'interrupt.*/
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

						/*se la syscall chiamata è superiore a 10 si richiama un gestore di trap se presente,
						altrimenti il processo corrente viene terminato e lo scheduler viene richiamato*/
						if(curr_proc->oldSYSBP != NULL && curr_proc->newSYSBP != NULL)
							SysBpTrapHandler();
						else if(curr_proc->oldTLB != NULL && curr_proc->newTLB != NULL)
							TlbTrapHandler();
						else if(curr_proc->oldPGT != NULL && curr_proc->newPGT != NULL)
							PgmTrapHandler();
						else{
							Terminate_Process(0);
							curr_proc = NULL;
							scheduler();
						}
						break;
				}

				curr_proc->p_s.reg_v0 = (U32) result;

				//time management:
				if(curr_proc->kernel_time_new > 0){
					curr_proc->kernel_time_old += getTODLO() - curr_proc->kernel_time_new;
					curr_proc->kernel_time_new = 0;
				}
				curr_proc->user_time_new = getTODLO();

				LDST(&curr_proc->p_s);
			break;

		case EXC_BREAKPOINT : //handler BREAKPOINT

				//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
				cpy_state((state_t*) SYSBK_OLDAREA, &curr_proc->p_s);

				//incremento il PC
				curr_proc->p_s.pc_epc += WORD_SIZE;

				//richiamo gestore di livello superiore
				SysBpTrapHandler();

			break;

		case EXC_RESERVEDINSTR :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_COPROCUNUSABLE :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_ARITHOVERFLOW :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			PgmTrapHandler();

			break;

		case EXC_BADPTE :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) TLB_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			TlbTrapHandler();

			break;

		case EXC_PTEMISS :

			//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
			cpy_state((state_t*) TLB_OLDAREA, &curr_proc->p_s);

			//incremento il PC
			curr_proc->p_s.pc_epc += WORD_SIZE;

			//richiamo gestore di livello superiore
			TlbTrapHandler();

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

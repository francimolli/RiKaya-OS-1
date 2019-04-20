#include "handler.h"
#include "utils.h"
#include "myconst.h"
#include "scheduler.h"

void Handler() {
	
	//creo una variabile che contiene il codice dell'eccezione inizializzata
	int cause_code = CAUSE_EXCCODE(getCAUSE());
	
	switch(cause_code){
		case interrupt : //handler interrupt
				//copio stato
				cpy_state((state_t*) INTERRUPT_OLDAREA, &curr_proc->p_s);
				
				/*quando si affronta un interrupt lo si affronta con la kernel mode abilitata,
				la virtual memory disabilitata, come già settato in fase di inizializzazione*/

				//Per la phase1.5 basta questo if, per la phase2 sarà presente uno switch per identificare l'interrupt
				if(CAUSE_IP_GET(curr_proc->p_s.cause,INT_LOCAL_TIMER)){
					setTIMER(3000);
					scheduler();
				}

				LDST((state_t*) INTERRUPT_OLDAREA);
		
		case breakpoint : //handler BREAKPOINT

				//copio nello state_t del processo che ha ricevuto l'interrupt ciò che risiede nella old area
				cpy_state((state_t*) SYSBR_OLDAREA, &curr_proc->p_s);
				
		break;

		case syscall : //handler SYSCALL
				
				//copio nello state_t del processo che ha ricevuto l'interrupt ciò che risiede nella old area
				cpy_state((state_t*) SYSBR_OLDAREA, &curr_proc->p_s);
				
				//identificazione tipo di system call, reg_a0
				switch(curr_proc->p_s.reg_a0){
					case terminate_proc :
						kill_proc();
					break;
					default	:
						LDST(&curr_proc->p_s);
					break;
				}
		break;
	}
}

#include "handler.h"
#include "utils.h"
#include "myconst.h"
#include "scheduler.h"

void sysbrHandler() {
	//copio nello state_t del processo che ha ricevuto l'interrupt ciò che risiede nella old area
	cpy_state((state_t*) SYSBR_OLDAREA, &curr_proc->p_s);
	//creo una variabile che contiene il codice dell'eccezione inizializzata
	int cause_code = CAUSE_EXCCODE(curr_proc->p_s.cause);
	switch(cause_code){
		case syscall :
				//creo una variabile con il tipo di system call richiesta
				//int sys_call_request=;
				switch(curr_proc->p_s.reg_a0){
					case terminate_proc :
						kill_proc();
					break;
					default	:
						LDST(&curr_proc->p_s);
					break;
				}
		break;
		case breakpoint:
				//handler che andrà a gestire i BREAKPOINT
		break;
	}

}
void tlbHandler() {

}

void interruptHandler() {
	//funzione da rivedere , bisogna vedere bit a bit e non  trasformare in int il dato
	cpy_state((state_t*) INTERRUPT_OLDAREA,&curr_proc->p_s);
	//trovo l'id dell'interrupt e lo metto nella variabile int_id
	int int_id= CAUSE_INTERRUPT(curr_proc->p_s.cause);
	switch(int_id){
		case 2:
			scheduler();
	}

}

void trapHandler() {

}

#include "utils.h"

inline void cpy_state(state_t* source, state_t* dest){
	dest->entry_hi = source->entry_hi;
	dest->cause = source->cause;
	dest->status = source->status;
	dest->pc_epc = source->pc_epc;
	for(int i = 0; i < STATE_GPR_LEN; i++){
		dest->gpr[i] = source->gpr[i];
	}
	dest->hi = source->hi;
	dest->lo = source->lo;
}

inline void PGMtrap(){
	//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
	cpy_state((state_t*) PGMTRAP_OLDAREA, &curr_proc->p_s);

	//incremento il PC
	curr_proc->p_s.pc_epc += WORD_SIZE;

	//richiamo gestore di livello superiore
	PgmTrapHandler();
}

inline void TLBtrap(){
	//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
	cpy_state((state_t*) TLB_OLDAREA, &curr_proc->p_s);

	//incremento il PC
	curr_proc->p_s.pc_epc += WORD_SIZE;

	//richiamo gestore di livello superiore
	TlbTrapHandler();
}

inline void SYSBPtrap(){
	//copio nello state_t del processo che ha ricevuto la trap ciò che risiede nella old area
	cpy_state((state_t*) SYSBK_OLDAREA, &curr_proc->p_s);

	//incremento il PC
	curr_proc->p_s.pc_epc += WORD_SIZE;

	//richiamo gestore di livello superiore
	SysBpTrapHandler();
}

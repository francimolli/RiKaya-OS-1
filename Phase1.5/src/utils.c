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


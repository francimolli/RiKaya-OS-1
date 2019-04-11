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

/*
#define CAUSE_IP_MASK          0x0000ff00
#define CAUSE_IP(line)         (1U << (8 + (line)))
#define CAUSE_IP_BIT(line)     (8 + (line))

*/
//

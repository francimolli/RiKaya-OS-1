
#ifndef UTILS_H
#define UTILS_H
#include <umps/types.h>
#include "const_rikaya.h"
#include "scheduler.h"

void cpy_state(state_t* source, state_t* dest);
void PGMtrap();
void TLBtrap();
void SYSBPtrap();

#endif

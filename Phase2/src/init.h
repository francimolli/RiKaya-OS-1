#ifndef INIT_H
#define INIT_H

#include <umps/cp0.h>
#include <umps/libumps.h>
#include "const_rikaya.h"
#include "pcb.h"

pcb_t* allocAndSet (const memaddr m, int priorityVal);
void populateNewAreas();

#endif

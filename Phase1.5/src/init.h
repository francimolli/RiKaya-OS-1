#ifndef INIT_H
#define INIT_H

#include "pcb.h"
#include <umps/cp0.h>
#include "myconst.h"

pcb_t* allocAndSet (const memaddr m, int priorityVal, unsigned int sMask);

void populateNewAreas();

#endif

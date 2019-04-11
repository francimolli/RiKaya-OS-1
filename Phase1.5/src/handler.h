#ifndef HANDLER_H
#define HANDLER_H

#define CAUSE_EXCCODE(cause) (((cause) & 0x0000007C) >> 2)
#define CAUSE_INTERRUPT(cause) (((cause) & CAUSE_IP_MASK) >> 8)

#include <umps/libumps.h>
#include "const.h"
#include <umps/cp0.h>

void sysbrHandler();
void tlbHandler();
void trapHandler();
void interruptHandler();

#endif

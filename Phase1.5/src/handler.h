#ifndef HANDLER_H
#define HANDLER_H

#define CAUSE_EXCCODE(cause) (((cause) & 0x0000007C) >> 2)
#define CAUSE_INTERRUPT(cause) (((cause) & CAUSE_IP_MASK) >> 8)
#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8))) //And tra cause e 1, shiftato di n posizioni

#include <umps/libumps.h>
#include "const.h"
#include <umps/cp0.h>

void sysbrHandler();
void tlbHandler();
void trapHandler();
void interruptHandler();

#endif

#ifndef HANDLER_H
#define HANDLER_H

#define CAUSE_EXCCODE(cause) (((cause) & 0x0000007C) >> 2)
//#define CAUSE_INTERRUPT(cause) (((cause) & CAUSE_IP_MASK) >> 8)
#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8))) //Faccio un and tra vero e il registro cause e shifto di n posizioni

#include <umps/libumps.h>
#include "const.h"
#include <umps/cp0.h>

void Handler();

#endif

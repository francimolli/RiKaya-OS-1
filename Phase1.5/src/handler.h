#ifndef HANDLER_H
#define HANDLER_H
#define CAUSE_EXCCODE(cause) (((cause) & 0x0000007C) >> 2)
#define CAUSE_INTERRUPT(cause) ((cause) & CAUSE_IP_MASK) >> 8)
#include <umps/libumps.h>
void sysbrHandler();
void tlbHandler();
void trapHandler();
void interruptHandler();

#endif

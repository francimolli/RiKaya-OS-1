#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "scheduler.h"
#include "syscalls.h"

int semDevices[MAX_DEVICES];
pcb_t *woken_proc;//processo risvegliato
U32 *dev_status;

void Interrupt_Handler();
int getDevice(int line_no, int dev_no);
void IO_request(U32 command, U32 * reg, U32 term_command);

#endif

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "scheduler.h"
#include "syscalls.h"

int semDevices[MAX_DEVICES];
pcb_t *woken_proc;//processo risvegliato
int *dev_status;

void Interrupt_Handler();
int getDevice(int line_no, int dev_no);
void IO_request(U32 command, U32 *register, U32 term_command);

#endif

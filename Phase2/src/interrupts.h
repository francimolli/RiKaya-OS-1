#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "scheduler.h"
#include "syscalls.h"
#include "handler.h"

int semDev[MAX_DEVICES];//array utilizzato per inizializzare i semafori dei devices correttamente
semdev semDevices;//semdev è la struttura dati contenente i semafori dei devices
semd_t pseudoclock_sem;//semaforo della wait clock
pcb_t *wakeup_proc;//processo risvegliato da una V quando un device completa l'operazione di I/O

void Interrupt_Handler();
int getDevice(int line_no, int dev_no);

#endif

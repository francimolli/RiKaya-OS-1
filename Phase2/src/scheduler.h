#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <umps/libumps.h>
#include "pcb.h"
#include "asl.h"
#include "types_rikaya.h"
#include "const_rikaya.h"
#include "syscalls.h"
#include "handler.h"

/*dichiariamo qui la ready_queue in modo che sia visibile
dallo scheduler ma possa venir inizializzata nel main*/
struct list_head ready_queue_h;
pcb_t *curr_proc;
int ProcBlocked;//integer per contare i processi bloccati su un semaforo

void scheduler();
void context();

#endif

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "pcb.h"
#include "myconst.h"

void scheduler();
	//dichiariamo qui la ready_queue in modo che sia visibile dallo scheduler ma possa venir 		inizializzata dal main
struct list_head ready_queue_h;
void kill_proc();

#endif

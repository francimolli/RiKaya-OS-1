#ifndef _TYPES11_H
#define _TYPES11_H

#include <umps/types.h>
#include <listx.h>

typedef unsigned int memaddr;

// Process Control Block (PCB) data structure
typedef struct pcb_t {
	/*process queue fields */
	struct list_head	p_next;

	/*process tree fields */
	struct pcb_t		*p_parent;
	struct list_head	p_child,
										p_sib;

	/* processor state, etc */
	state_t       		p_s;

	/* process priority */
	int         original_priority;
  int					priority;

	/*process time of execution*/
	int 				user_time_old,
							user_time_new,
							kernel_time_old,
							kernel_time_new,
							wallclock_time;

	/*tutor field*/
	int 				tutor;

	/*io command field*/
	int 				command,
							recv_or_transm;

	/*spec_passup fields*/
	state_t *		oldSYSBP;
	state_t * 		newSYSBP;
	state_t *		oldTLB;
	state_t *		newTLB;
	state_t *		oldPGT;
	state_t *		newPGT;

	/* key of the semaphore on which the process is eventually blocked */
	int			*p_semkey;
} pcb_t;



// Semaphore Descriptor (SEMD) data structure
typedef struct semd_t {
	struct list_head	s_next;

	// Semaphore key
	int    *s_key;

	// Queue of PCBs blocked on the semaphore
	struct list_head	s_procQ;
} semd_t;

#endif

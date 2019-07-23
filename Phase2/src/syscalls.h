#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "const_rikaya.h"
#include "scheduler.h"
#include "interrupts.h"
#include "utils.h"

void Get_CPU_Time(U32 *user, U32 *kernel, U32 *wallclock);
int Create_Process(state_t *statep, int priority, void **cpid);
int Terminate_Process(void **pid);
int lookup_proc(pcb_t *p, pcb_t *q);
void Verhogen(int *semaddr);
void Passeren(int *semaddr);
void Wait_Clock();
int Do_IO(U32 command, U32 *reg, U32 term_command);
void Set_Tutor();
int Spec_Passup(int type, state_t *old, state_t *new);
void Get_pid_ppid(void **pid, void **ppid);



#endif

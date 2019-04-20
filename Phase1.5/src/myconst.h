#ifndef MYCONST_H
#define MYCONST_H

#define SYSBR_NEWAREA     0x200003d4
#define SYSBR_OLDAREA     0x20000348
#define TLB_NEWAREA       0x200001a4
#define TLB_OLDAREA       0x20000118
#define INTERRUPT_NEWAREA 0x2000008c
#define INTERRUPT_OLDAREA 0x20000000
#define PGMTRAP_NEWAREA   0x200002bc
#define PGMTRAP_OLDAREA   0x20000230

#define TOD_LO *((unsigned int *)0x1000001C)
#define TIME_SCALE *((unsigned int *)0x10000024)
#define RAMBASE *((unsigned int *)0x10000000)
#define RAMSIZE *((unsigned int *)0x10000004)
#define RAMTOP (RAMBASE + RAMSIZE)
#define FRAMESIZE 4096

#define interrupt 0
#define terminate_proc 3
#define syscall 8
#define breakpoint 9


#define CP0_EN_MASK  0x10000000 //Abilita la CP0
#define TMR_EN_MASK  0x08000000 //Abilita il timer
#define KM_EN_MASK   0xFFFFFFFD //Abilita la Kernel mode
#define VM_DIS_MASK  0xFEFFFFFF //Disabilita la memoria virtuale
#define INT_DIS_MASK 0xFFFF00FA //Disabilita gli interrupt (anche IEp)
#define E_CODE_MASK  0x7C //Maschera per leggere la causa di un'exception

/*Interrupt codes*/
#define INT_LOCAL_TIMER 1 //Processor local time
#define INT_TIMER 2    //Interval timer
#define INT_LOWEST 3   //Minimum interrupt number used by real devices
#define INT_DISK 3
#define INT_TAPE 4
#define INT_UNUSED 5
#define INT_PRINTER 6
#define INT_TERMINAL 7

#endif

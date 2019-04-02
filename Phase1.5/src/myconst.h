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

#endif

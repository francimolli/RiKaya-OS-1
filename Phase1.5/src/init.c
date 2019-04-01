#include "init.h"

pcb_t* allocAndSet (const memaddr m, int priorityVal, unsigned int sMask) {

    pcb_t* p;

    static int SPn = 1;

    p = allocPcb();

    p->p_s.pc_epc = m;
    p->priority = priorityVal;
    
    
    //Interrupt abilitati
    //p->p_s.status |=  STATUS_IEc ;
    //Processo in Kernel Mode
    //p->p_s.status |= ~STATUS_VMc ;
    //Virtual Memory OFF
    //p->p_s.status |= ~STATUS_KUc ;
    //Processor Local Timer Abilitato
    //p->p_s.status |=  STATUS_TE ;
    //Inizializzare SP
    
    p->p_s.status |= sMask;
    
    //p->p_s.reg_sp = RAMTOP - FRAMESIZE * SPn ;
    SPn = SPn + 1 ;
    
    return p;

}


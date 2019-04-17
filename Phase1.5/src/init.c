#include "init.h"
#include <umps/libumps.h>

//ricavo gli indirizzi relativi alle funzioni che si occuperanno, rispettivamente di gestire:
//SystemCall&Breakpoint - TLB - Trap - Interrupt
extern void sysbrHandler();
extern void tlbHandler();
extern void trapHandler();
extern void interruptHandler();

#define sysbrHandlerAddress ((memaddr)sysbrHandler)
#define tlbHandlerAddress ((memaddr)tlbHandler)
#define trapHandlerAddress ((memaddr)trapHandler)
#define interruptHandlerAddress ((memaddr)interruptHandler)

//funzione che alloca ed inizializza un PCB, prendendo come valori :
//memaddr m : entry point della funzione che il processo dovrà eseguire
//int priorityVal : priorità iniziale del processo
//uint sMask : maschera di status
pcb_t* allocAndSet (const memaddr m, int priorityVal, unsigned int sMask) {

    pcb_t* p;



    p = allocPcb();
    if (p==NULL){

      HALT();
    }
    //set PC
    p->p_s.pc_epc = p->p_s.reg_t9 = m;
    //set priority
    p->priority = p->original_priority = priorityVal;


    //Interrupt abilitati
    //p->p_s.status |=  STATUS_IEc ;
    //Processo in Kernel Mode
    //p->p_s.status |= ~STATUS_VMc ;
    //Virtual Memory OFF
    //p->p_s.status |= ~STATUS_KUc ;
    //Processor Local Timer Abilitato
    //p->p_s.status |=  STATUS_TE ;
    //Inizializzare SP

    //sMask è una maschera di status impostata come qui sopra ^

    p->p_s.status |= (~INT_DIS_MASK | TMR_EN_MASK | CP0_EN_MASK); //Linee di interrupt e PLT
    p->p_s.status &= (KM_EN_MASK & VM_DIS_MASK & 0xFFFF7FFF);//abilito la kernel mode disabilito la virtual memory e disabilito tutti gli interrupt meno che il 15
    p->p_s.reg_sp = RAMTOP - (FRAMESIZE * priorityVal) ;


    return p;

}

#define defaultNewAreaMask (((~STATUS_IEc & ~STATUS_VMc) | STATUS_KUc) | STATUS_TE)
//funzione che popola un'area, prende in input l'indirizzo dell'area da popolare
//e l'indirizzo dell'handler che si occupa di gestire l'eccezione
//inizializza inoltre lo status nel seguente modo:
//-Interrupt e VM disabilitati - KernelMode ON - Timer abilitato
//lo SP punta a RAMTOP
HIDDEN void populateArea (state_t* areaAddress, memaddr handlerFun) {

  //  areaAddress->status = defaultNewAreaMask ;
    areaAddress->cause=0;
    areaAddress->reg_sp = RAMTOP ;
    areaAddress->pc_epc = areaAddress->reg_t9 = handlerFun ;
    areaAddress->status &= (KM_EN_MASK & VM_DIS_MASK & INT_DIS_MASK & ~TMR_EN_MASK);
    areaAddress->status |= CP0_EN_MASK;
}

//funzione che popola le 4 New Areas
void populateNewAreas () {

    populateArea ((state_t*)SYSBR_NEWAREA     , sysbrHandlerAddress); //SYS,BR
    populateArea ((state_t*)TLB_NEWAREA       , tlbHandlerAddress); //TLB
    populateArea ((state_t*)INTERRUPT_NEWAREA , interruptHandlerAddress); //INTERRUPT
    populateArea ((state_t*)PGMTRAP_NEWAREA   , trapHandlerAddress); //TRAP
    setTIMER(3000);
}

#include "init.h"

/*Ricavo l'indirizzo relativo alla funzione che si occupa di gestire:
SystemCall&Breakpoint - TLB - Trap - Interrupt*/

extern void Handler();

#define HandlerAddress ((memaddr)Handler)

/*Funzione che popola un'area, prende in input l'indirizzo dell'area da popolare
e l'indirizzo dell'handler che si occupa di gestire l'eccezione
inizializza inoltre lo status nel seguente modo:
-Interrupt e VM disabilitati - KernelMode ON - Timer abilitato
lo SP punta a RAMTOP*/
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

    populateArea ((state_t*)SYSBK_NEWAREA     , HandlerAddress); //SYS,BR
    populateArea ((state_t*)TLB_NEWAREA       , HandlerAddress); //TLB
    populateArea ((state_t*)INT_NEWAREA       , HandlerAddress); //INTERRUPT
    populateArea ((state_t*)PGMTRAP_NEWAREA   , HandlerAddress); //TRAP
}

/*Funzione che alloca ed inizializza un PCB, prendendo come valori :
memaddr m : entry point della funzione che il processo dovrà eseguire
int priorityVal : priorità iniziale del processo*/
pcb_t* allocAndSet (const memaddr m, int priorityVal) {

    pcb_t* p;
    p = allocPcb();

    if (p==NULL){
      HALT();
    }

    //set PC
    p->p_s.pc_epc = p->p_s.reg_t9 = m;
    //set priority
    p->priority = p->original_priority = priorityVal;
    //Linee di interrupt e PLT
    p->p_s.status |= (~INT_DIS_MASK | TMR_EN_MASK | CP0_EN_MASK);
    //abilito la kernel mode disabilito la virtual memory e disabilito tutti gli interrupt meno che il 15
    p->p_s.status &= (KM_EN_MASK & VM_DIS_MASK & 0xFFFF7FFF);
    //set SP
    p->p_s.reg_sp = RAMTOP - (FRAME_SIZE * priorityVal) ;

    return p;
}

void initSemDevices(){

  for(int i = 0; i < MAX_DEVICES; i++)
    semDev[i] = 1;

  for(int i = 0; i < DEV_PER_INT; i++){

   semDevices.disk[i].s_key = &semDev[i];

   semDevices.tape[i].s_key = &semDev[i + DEV_PER_INT];

   semDevices.network[i].s_key = &semDev[i + 2 * DEV_PER_INT];

   semDevices.terminalR[i].s_key = &semDev[i + 3 * DEV_PER_INT];

   semDevices.terminalT[i].s_key = &semDev[i + 4 * DEV_PER_INT];
  }
  semDevices.pseudoclock.s_key = &semDev[CLOCK_SEM];
}

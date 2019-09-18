#include "traps.h"

void SysBpTrapHandler(){

    if(curr_proc->newSYSBP == NULL){

      //non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
      Terminate_Process(0);

      curr_proc = NULL;

      scheduler();
    }
    else{

      /*c'è un gestore di livello superiore di tipo SYSBP, perciò si copia nell'old area lo stato del processo
      corrente e si carica nel curr_proc il codice della new area.*/
      cpy_state(&curr_proc->p_s, curr_proc->oldSYSBP);
      LDST(curr_proc->newSYSBP);
    }
}

void TlbTrapHandler(){

  if(curr_proc->newTLB == NULL){

    //non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
    Terminate_Process(0);

    curr_proc = NULL;

    scheduler();
  }
  else{

    /*c'è un gestore di livello superiore di tipo TLB, perciò si copia nell'old area lo stato del processo
    corrente e si carica nel curr_proc il codice della new area.*/
    cpy_state(&curr_proc->p_s, curr_proc->oldTLB);
    LDST(curr_proc->newTLB);
  }

}
void PgmTrapHandler(){

  if(curr_proc->newPGT == NULL){

    //non c'è un puntatore ad un gestore di livello superiore, e quindi il processo va terminato
    Terminate_Process(0);

    curr_proc = NULL;

    scheduler();
  }
  else{

    /*c'è un gestore di livello superiore di tipo TLB, perciò si copia nell'old area lo stato del processo
    corrente e si carica nel curr_proc il codice della new area.*/
    cpy_state(&curr_proc->p_s, curr_proc->oldPGT);
    LDST(curr_proc->newPGT);
  }
}

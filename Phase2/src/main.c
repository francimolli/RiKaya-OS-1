#include "const_rikaya.h"
#include "listx.h"
#include <umps/libumps.h>
#include <umps/arch.h>

#include "pcb.h"
#include "init.h"
#include "scheduler.h"
#include "syscalls.h"
#include "interrupts.h"
#include "handler.h"

extern void test();

int main () {

    pcb_t *proc;

    //Popolo le New Areas nel ROM Reserved Frame
    populateNewAreas();

    //Istanzio la lista dei PCB
    initPcbs();

    //Istanzio la lista dei semafori
    initASL();

    //Inizializzo i semafori relativi ai devices e allo pseudo-clock
    initSemDevices();

    //Inizializzo il puntatore al processo corrente
    curr_proc = NULL;

    //Inizializzo il contatore di processi bloccati
    ProcBlocked = 0;

    /*Alloco il processo di test, inizializzando il PCB relativo ed assegnandoli
    puntatore all'area di memoria della sua funzione*/
    proc = allocAndSet ((memaddr)test, DEFAULT_PRIORIY);

    //il processo iniziale deve sempre essere il tutor per i processi orfani
    proc->tutor = TRUE;

    mkEmptyProcQ(&ready_queue_h);

    //inserisco il PCB con l'indirizzo della funzione di test nella lista dei PCB
    insertProcQ(&ready_queue_h, proc);

    /*Si fa partire il timer per lo pseudoclock (UL serve a garantire che quando avviene
    la moltiplicazione con BUS_TIMESCALE non si verifichi un overflow)*/
    SET_IT(CLOCKINTERVAL);

    //inizio esecuzione
    scheduler();

    return 0;
}

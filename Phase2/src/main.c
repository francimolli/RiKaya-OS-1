#include "const_rikaya.h"
#include "listx.h"
#include <umps/libumps.h>
#include <umps/arch.h>

#include "pcb.h"
#include "init.h"
#include "scheduler.h"
#include "syscalls.h"
#include "interrupts.h"

pcb_t *proc;

extern void addokbuf(char *strp);
extern void test();

int main () {

    //Popolo le New Areas nel ROM Reserved Frame
    populateNewAreas();
    addokbuf("NEW AREAS popolate\n");

    //Istanzio la lista dei PCB
    initPcbs();
    addokbuf("Process Control Blocks inizializzati\n");

    //Istanzio la lista dei semafori
    initASL();
    addokbuf("ASL inizializzata\n");

    //Inizializzo i semafori relativi ai devices e allo pseudo-clock
    for(int i = 0; i < MAX_DEVICES; i++)
	   semDevices[i] = 0;

    /*Alloco il processo di test, inizializzando il PCB relativo ed assegnandoli
    puntatore all'area di memoria della sua funzione*/
    proc = allocAndSet ((memaddr)test, 1);

    addokbuf("PCB funzione test allocato\n");

    mkEmptyProcQ (&ready_queue_h);

    //inserisco i 3 PCB precedentemente creati nella lista dei PCB
    insertProcQ (&ready_queue_h, proc);
    //salvataggio delle priorità iniziali nel campo original_priority

    addokbuf("Inizio esecuzione\n");

    //inizio esecuzione
    scheduler();

    return 0;
}

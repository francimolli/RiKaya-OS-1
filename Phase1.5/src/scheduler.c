#include "scheduler.h"



/*
 * In questa fase, il sistema operativo entra nell'entry point (il main) 
 * dove vengono inizializzate le varie strutture e dopodichè passa il testimone
 * allo scheduler che si occupa totalmente da quel momento in poi della gestione
 * dei processi. Per questa fase ipotizziamo che venga, fuori dallo scheduler (nel main)
 * inizializzata una coda di processi pronti (test1, test2, test3) che passiamo come parametro
 * allo scheduler, il quale si occuperà della loro corretta esecuzione.
 * Idealmente, lo scheduler dovrebbe permettere l'aggiunta a runtime di ulteriori processi
 * nella coda dei processi pronti, ma questo richiederebbe un (INTERRUPT o SYSCALL?) da parte
 * del sistema operativo ad informare lo scheduler della presenza di un nuovo processo.
 * Per questa fase ci limitiamo a gestire i 3 processi di test.
*/


//readyQueue_h = sentinella gestita dallo scheduler relativa alla coda dei processi pronti

void scheduler (struct list_head* readyQueue_h) {


}

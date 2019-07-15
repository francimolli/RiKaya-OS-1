#include "interrupts.h"

void Interrupt_Handler(){

  /*per capire quale (o quali) interrupt sta pendendo è necessario controllare
  ciascun bit del campo IP del registro CAUSE. Si utilizza la macro CAUSE_IP_GET
  per verificare se l'i-esimo bit di IP vale 1.*/

  devreg_t *dev;//puntatore al registro del device che ha sollevato l'interrupt (può essere dtpreg_t/termreg_t)


  //Interrupt Processor Local Timer
  if(CAUSE_IP_GET(curr_proc->p_s.cause, INT_T_SLICE)){
      setTIMER(TIME_SLICE);
      scheduler();
  }

  //Interrupt Interval Timer
  if(CAUSE_IP_GET(curr_proc->p_s.cause, INT_TIMER)){
     /*quando si solleva questo interrupt, allora tutti
     i processi bloccati sul semaforo della syscall Wait_Clock
     sono risvegliati*/
     /*versione 1
     pcb_t *p = curr_proc;
     while(p != NULL){
       p = removeBlocked(&wait_clock_sem_key);
       insertProcQ(&ready_queue_h, p);
     }*/
     int *semaddr = &semDevices[CLOCK_SEM];
     while(*semaddr <= 0){
       Verhogen(semaddr);
     }

     SET_IT(TIME_SLICE);//acknowledgement scrivendo un nuovo valore all'interno dell'interval timer

  }

  //Interrupt Disk, Tape, Network, Printer
  for(int j = INT_LOWEST; j < 7; j++){
    if(CAUSE_IP_GET(curr_proc->p_s.cause, j)){

      for(int i = 0; i < DEV_PER_INT; i++){

        //controllo se il device i di tipo j ha un interrupt pendente
        if(getDevice(j, i)){

          /*ottengo il puntatore al registro del device:
          i *  4 è l'offset all'interno della zona di memoria (4 perchè il registro del device
          è formato da 4 campi) dedicata al Device di tipo j, mentre (j - 3) * DEV_PER_INT * 4
          è l'inizio della zona di memoria dedicata al tipo di device j che può essere disk,
          tape, network, printer.*/

          dev = (dtpreg_t *) (DEV_REGS_START + (i * 4) + (j - 3) * DEV_PER_INT * 4);
          dev_status = (int *)dev;

          /*il processo corrente richiede un'operazione di I/O su questo device con la syscall
          Do_IO, perciò esso deve essere bloccato fino a quando l'operazione di I/O non è terminata
          (infatti nella Do_IO viene fatta una Passeren).
          Innanzitutto è necesssario controllare che il device sia libero e non occupato
          (ovvero il relativo semaforo ha valore 0).
          Si fa, dunque, una P sul semaforo del device ì (la P viene fatta da Do_IO), e quando
          si solleva l'interrupt perchè l'operazione è stata completata, viene fatta la V, inoltre viene
          scritto l'ack nel registro del device in modo da concludere l'operazione e farne partire un'altra
          immediatamente*/

          if(semDevices[(j - 3) * DEV_PER_INT + i] < 0){
            woken_proc = removeBlocked(&semDevices[(j - 3) * DEV_PER_INT + i]);
            semDevices[(j - 3) * DEV_PER_INT + i]++;
            woken_proc->p_s.reg_v0 = (dtpreg_t *)dev->status;
            insertProcQ(&ready_queue_h, woken_proc);
          }

          dev->command = DEV_C_ACK;//acknowledgement
          //attesa completamento richiesta di I/O
          while(dev->status != DEV_S_READY)
            ;

          //parte immediatamente un'altra operazione (se c'è)
          if(semDevices[(j - 3) * DEV_PER_INT + i] < 0){
            woken_proc = removeBlocked(&semDevices[(j - 3) * DEV_PER_INT + i]);
            semDevices[(j - 3) * DEV_PER_INT + i]++;
            IO_request(woken_proc->command, dev, 0);
          }
        }
      }
    }
  }

  //Interrupt Terminal
  if(CAUSE_IP_GET(curr_proc->p_s.cause, INT_TERMINAL)){

    //controllo quale terminale ha causato l'interrupt
    for(int i = 0; i < DEV_PER_INT; i++){

        if(getDevice(7, i)){

          /*Come fatto per i device disk, tape, network e printer, si ottiene
          l'indirizzo base da cui inizia l'area di memoria dedicata al terminale i.*/

          dev = (termreg_t *) (DEV_REGS_START + (i * 4) + 4 * DEV_PER_INT * 4);
          dev_status = (int *)dev;

          /*ottenuto l'indirizzo base è necessario distinguere se l'interrupt sia stato sollevato dal
          sub-device in ricezione o trasmissione, o entrambi (se l'interrupt è stato causato da entrambi,
          allora c'è più di un solo interrupt pendente attaccato alla linea del terminale ed è neccessario
          che entrambi gli interrupt ricevano l'acknowledgement).
          Per fare ciò è necessario andare a leggere il contenuto del campo recv_status e transm_status.
          In particolare in ciascuno dei due campi, nello status byte, ad operazione di
          ricezione/trasmissione completata, si legge se il device è ready.
          Dunque si legge lo status byte facendo un & bitwise con la statusmask, 0xFF presente nel file const_rikaya.*/

          if(semDevices[4*DEV_PER_INT + i] < 0 &&
             (dev->recv_status & STATUSMASK) == DEV_TRCV_S_CHARRECV &&
             (dev->transm_status & STATUSMASK) == DEV_TTRS_S_CHARTRSM){

               //caso in cui il terminale i ha sollevato un interrupt sia per ricezione che per trasmissione
               woken_proc = removeBlocked(&semDevices[4*DEV_PER_INT + i]);
               semDevices[4*DEV_PER_INT + i]++;
               woken_proc->p_s.reg_v0 = dev->recv_status; //indifferente che sia recv_status o transm_status
               insertProcQ(&ready_queue_h, woken_proc);

               dev->recv_command = DEV_C_ACK;
               dev->transm_command = DEV_C_ACK;

               while((dev->recv_status & STATUSMASK) != DEV_S_READY || (dev->transm_status & STATUSMASK) != DEV_S_READY)
                ;
          }

          if(semDevices[4*DEV_PER_INT + i] < 0 &&
             (dev->recv_status & STATUSMASK) == DEV_TRCV_S_CHARRECV &&
             (dev->transm_status & STATUSMASK) == DEV_S_READY){

                //caso in cui il terminale i ha sollevato un interrupt solo per la ricezione
                woken_proc = removeBlocked(&semDevices[4*DEV_PER_INT + i]);
                semDevices[4*DEV_PER_INT + i]++;
                woken_proc->p_s.reg_v0 = dev->recv_status;
                insertProcQ(&ready_queue_h, woken_proc);

                dev->recv_command = DEV_C_ACK;

                while((dev->recv_status & STATUSMASK) != DEV_S_READY)
                  ;
          }

          if(semDevices[4*DEV_PER_INT + i] < 0 &&
             (dev->transm_status & STATUSMASK) == DEV_TTRS_S_CHARTRSM &&
             (dev->recv_status & STATUSMASK) == DEV_S_READY){

                //caso in cui il terminale i ha sollevato un interrupt solo per la ricezione
                woken_proc = removeBlocked(&semDevices[4*DEV_PER_INT + i]);
                semDevices[4*DEV_PER_INT + i]++;
                woken_proc->p_s.reg_v0 = dev->transm_status;
                insertProcQ(&ready_queue_h, woken_proc);

                dev->transm_command = DEV_C_ACK;

                while((dev->transm_status & STATUSMASK) != DEV_S_READY)
                  ;
          }


          if(semDevices[4*DEV_PER_INT + i] < 0){

            //prossima operazione
            woken_proc = removeBlocked(&semDevices[4*DEV_PER_INT + i]);
            semDevices[4*DEV_PER_INT + i]++;
            if(woken_proc->recv_or_transm)
              IO_request(woken_proc->command, dev, TRUE);
            else
              IO_request(woken_proc->command, dev, FALSE);
          }
        }
    }
  }
}

int getDevice(int line_no, int dev_no){

  /*questa funzione ritorna 1 se il device i attaccato alla linea j solleva un interrupt.
  Per identificare il device i si sfrutta l'indirizzo di PENDING_BITMAP_START, ovvero
  l'indirizzo in memoria dove inizia la Interrupting Devices Bit Map*/

  line_no -= 3;

  /*una parola è riservata in memoria per indicare quale device ha interrupts pendenti
  sulle linee da 3 a 7. Quindi se si tratta del terminale per esempio, è necessario spostarsi
  di 7 - 3 = 4 parole in avanti da PENDING_BITMAP_START. Successivamente è necessario fare uno shift
  a destra di dev_no posizioni per vedere se il device dev_no associato a line_no ha un interruptpendente.
  Dopo lo shift si fa un & bitwise con 0x00000001 in modo da vedere se effettivamente il bit destinato a
  dev_no è 1. */

  //se non va prova ad usare le macro in fondo al file const_rikaya
  if(((PENDING_BITMAP_START + line_no) >> dev_no) & 0x00000001)
    return 1;

  return 0;
}

void IO_request(U32 command, U32 *register, U32 term_command){

  int *semaddr;
  /*offset per ottenere il semaforo associato al device del registro register (4 è il numero di campi
  che compongono un area di memoria dedicata ad un device)*/
  U32 sem = (register - DEV_REGS_START)/4;

  semaddr = &semDevices[sem];

  /*è necessario distinguere se la richiesta di I/O è per un disk/tape/network/printer oppure
  per un terminal. Nel caso la richiesta sia del terminal, allora il registro associato al
  puntatore register sarà ad una locazione di memoria che sta almeno a distanza
  4*DEV_PER_INT*4, ovvero il numero di locazioni destinate ai device disk, tape, network, printer,
  da DEV_REGS_START. Dunque l'offset (con cui andare a cercare il semaforo dentro l'array di semafori
  semDevices) calcolato prima (sem) deve aver un valore superiore a 4*DEV_PER_INT ma minore di
  MAX_DEVICES - 1 (si consideri che l'offset parte da 0).*/

  if(sem > 31 && sem < 40){

    /*il registro puntato da register è quello di un terminale perciò bisogna capire se andare a scrivere
    il comando all'interno del campo RECV_COMMAND oppure all'interno di TRANSM_COMMAND.
    Inoltre l'handshake inizia con la scrittura di un command code all'interno del campo command del registro*/
    if(term_command == FALSE){

      /*il parametro command è da scrivere nel campo TRANSM_COMMAND, ovvero si tratta
      di un'operazione di trasmissione su un terminale*/
      ((termreg_t *)register)->transm_command = command;
    }
    else{
      /*il parametro command è da scrivere nel campo RECV_COMMAND, ovvero si tratta
      di un'operazione di ricezione su un terminale*/
      ((termreg_t *)register)->recv_command = command;
    }

  }
  else{

    //l'handshake inizia con la scrittura di un command code all'interno del campo command del registro
    ((dtpreg_t *)register)->command = command;
  }

  if(woken_proc == curr_proc)
    Passeren(semaddr);
  else{
    *semaddr--;
    insertBlocked(semaddr, woken_proc);
    outProcQ(&ready_queue_h, woken_proc);
  }
}

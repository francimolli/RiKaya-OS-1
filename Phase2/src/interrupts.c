#include "interrupts.h"

void Interrupt_Handler(){

  /*per capire quale (o quali) interrupt sta pendendo è necessario controllare
  ciascun bit del campo IP del registro CAUSE. Si utilizza la macro CAUSE_IP_GET
  per verificare se l'i-esimo bit di IP vale 1.*/

  U32 cause = old_area->cause;

  dtpreg_t *dev;//puntatore al registro del device che ha sollevato l'interrupt
  termreg_t *term;//puntatore al registro del terminale che ha sollevato l'interrupt

  //Interrupt Processor Local Timer
  if(CAUSE_IP_GET(cause, INT_T_SLICE)){

      setTIMER(TIME_SLICE);
      scheduler();
  }

  //Interrupt Interval Timer
  if(CAUSE_IP_GET(cause, INT_TIMER)){
     /*quando si solleva questo interrupt, allora tutti
     i processi bloccati sul semaforo della syscall Wait_Clock
     sono risvegliati*/
     /*versione 1
     pcb_t *p = curr_proc;
     while(p != NULL){
       p = removeBlocked(&wait_clock_sem_key);
       insertProcQ(&ready_queue_h, p);
     }*/

     int *semaddr = semDevices.pseudoclock.s_key;
     while(*semaddr < 0){
       Verhogen(semaddr);
     }

     SET_IT(100);//acknowledgement scrivendo un nuovo valore all'interno dell'interval timer

  }

  //NB usa indirizzi per distinguere che dev fa IO

  //Disk Interrupt
  if(CAUSE_IP_GET(cause, INT_DISK)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_DISK ha un interrupt pendente
      if(getDevice(INT_DISK, i)){

        /*ottengo il puntatore al registro del device:
        i *  4 è l'offset all'interno della zona di memoria (4 perchè il registro del device
        è formato da 4 campi) dedicata al Device di tipo j, mentre (j - 3) * DEV_PER_INT * 4
        è l'inizio della zona di memoria dedicata al tipo di device j che può essere disk,
        tape, network, printer.*/

        dev = (dtpreg_t *) (DISK_START + i * DEV_REG_SIZE_W);


        /*il processo corrente richiede un'operazione di I/O su questo device con la syscall
        Do_IO, perciò esso deve essere bloccato fino a quando l'operazione di I/O non è terminata
        (infatti nella Do_IO viene fatta una Passeren).
        Innanzitutto è necesssario controllare che il device sia libero e non occupato
        (ovvero il relativo semaforo ha valore 0).
        Si fa, dunque, una P sul semaforo del device ì (la P viene fatta da Do_IO), e quando
        si solleva l'interrupt perchè l'operazione è stata completata, viene fatta la V, inoltre viene
        scritto l'ack nel registro del device in modo da concludere l'operazione e farne partire un'altra
        immediatamente*/

        if(*semDevices.disk[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.disk[i].s_key);
          (*semDevices.disk[i].s_key)++;
          ProcBlocked--;
          wakeup_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, wakeup_proc);
        }

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.disk[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.disk[i].s_key);
          (*semDevices.disk[i].s_key)++;
          Do_IO(wakeup_proc->command, (U32 *) dev, 0);
        }
        else dev->command = DEV_C_ACK;//acknowledgement se non c'è un'altra operazione da fare
      }
    }
  }

  //Tape Interrupt
  if(CAUSE_IP_GET(cause, INT_TAPE)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_TAPE ha un interrupt pendente
      if(getDevice(INT_TAPE, i)){

        dev = (dtpreg_t *) (TAPE_START + (i * DEV_REG_SIZE_W));


        if(*semDevices.tape[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.tape[i].s_key);
          (*semDevices.tape[i].s_key)++;
          ProcBlocked--;
          wakeup_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, wakeup_proc);
        }

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.tape[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.tape[i].s_key);
          (*semDevices.tape[i].s_key)++;
          Do_IO(wakeup_proc->command, (U32 *) dev, 0);
        }
        else dev->command = DEV_C_ACK;//acknowledgement se non c'è un'altra operazione da fare
      }
    }
  }

  //Network Interrupt
  if(CAUSE_IP_GET(cause, INT_UNUSED)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_UNUSED(network) ha un interrupt pendente
      if(getDevice(INT_UNUSED, i)){

        dev = (dtpreg_t *) (NETWORK_START + (i * DEV_REG_SIZE_W));


        if(*semDevices.network[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.network[i].s_key);
          (*semDevices.network[i].s_key)++;
          ProcBlocked--;
          wakeup_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, wakeup_proc);
        }

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.network[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.network[i].s_key);
          (*semDevices.network[i].s_key)++;
          Do_IO(wakeup_proc->command, (U32 *) dev, 0);
        }
        else dev->command = DEV_C_ACK;//acknowledgement se non c'è un'altra operazione da fare
      }
    }
  }


  //Printer Interrupt
  if(CAUSE_IP_GET(cause, INT_PRINTER)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_PRINTER ha un interrupt pendente
      if(getDevice(INT_PRINTER, i)){

        dev = (dtpreg_t *) (PRINTER_START + (i * DEV_REG_SIZE_W));


        if(*semDevices.printer[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.printer[i].s_key);
          (*semDevices.printer[i].s_key)++;
          ProcBlocked--;
          wakeup_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, wakeup_proc);
        }

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.printer[i].s_key < 0){
          wakeup_proc = removeBlocked(semDevices.printer[i].s_key);
          (*semDevices.printer[i].s_key)++;
          Do_IO(wakeup_proc->command, (U32 *) dev, 0);
        }
        else dev->command = DEV_C_ACK;//acknowledgement se non c'è un'altra operazione da fare
      }
    }
  }

  //Terminal Interrupt
  if(CAUSE_IP_GET(cause, INT_TERMINAL)){

    //controllo quale terminale ha causato l'interrupt
    for(int i = 0; i < DEV_PER_INT; i++){

        if(getDevice(7, i)){

          /*Come fatto per i device disk, tape, network e printer, si ottiene
          l'indirizzo base da cui inizia l'area di memoria dedicata al terminale i.*/

          term = (termreg_t *) (TERM0ADDR + (i * DEV_REG_SIZE_W));

          /*ottenuto l'indirizzo base è necessario distinguere se l'interrupt sia stato sollevato dal
          sub-device in ricezione o trasmissione, o entrambi (se l'interrupt è stato causato da entrambi,
          allora c'è più di un solo interrupt pendente attaccato alla linea del terminale ed è neccessario
          che entrambi gli interrupt ricevano l'acknowledgement).
          Per fare ciò è necessario andare a leggere il contenuto del campo recv_status e transm_status.
          In particolare in ciascuno dei due campi, nello status byte, ad operazione di
          ricezione/trasmissione completata, si legge se il device è ready.
          Dunque si legge lo status byte facendo un & bitwise con la statusmask, 0xFF presente nel file const_rikaya.*/

          if((term->recv_status & STATUSMASK) != DEV_S_READY){

                if(*semDevices.terminalR[i].s_key < 0){
                  //caso in cui il terminale i ha sollevato un interrupt solo per la ricezione
                  wakeup_proc = removeBlocked(semDevices.terminalR[i].s_key);
                  (*semDevices.terminalT[i].s_key)++;
                  ProcBlocked--;
                  wakeup_proc->p_s.reg_v0 = term->recv_status;
                  insertProcQ(&ready_queue_h, wakeup_proc);
                }

            //prossima operazione
            if(*semDevices.terminalR[i].s_key < 0){

              wakeup_proc = removeBlocked(semDevices.terminalR[i].s_key);
              (*semDevices.terminalR[i].s_key)++;
              //chiamata di IO sul terminale in ricezione
              Do_IO(wakeup_proc->command,(U32 *) term, TRUE);
            }
            else dev->command = DEV_C_ACK;//acknowledgement se non c'è un'altra operazione da fare
          }

          if((term->transm_status & STATUSMASK) != DEV_S_READY){

              if(*semDevices.terminalT[i].s_key < 0){

                //caso in cui il terminale i ha sollevato un interrupt solo per la trasmissione
                wakeup_proc = removeBlocked(semDevices.terminalT[i].s_key);
                (*semDevices.terminalT[i].s_key)++;
                ProcBlocked--;
                wakeup_proc->p_s.reg_v0 = term->transm_status;

                insertProcQ(&ready_queue_h, wakeup_proc);
              }

              //NB niente verhogen perchè, la V non ritorna un pcb_t

              //prossima operazione
              if(*semDevices.terminalT[i].s_key < 0){

                wakeup_proc = removeBlocked(semDevices.terminalT[i].s_key);
                (*semDevices.terminalT[i].s_key)++;
                //chiamata di IO sul terminale in trasmissione
                Do_IO(wakeup_proc->command,(U32 *) term, FALSE);
              }
              else term->transm_command = DEV_C_ACK;
            }
        }

    }

  }

}

int getDevice(int line_no, int dev_no){

  /*questa funzione ritorna 1 se il device i attaccato alla linea j solleva un interrupt.
  Per identificare il device i si sfrutta l'indirizzo di PENDING_BITMAP_START, ovvero
  l'indirizzo in memoria dove inizia la Interrupting Devices Bit Map*/

  /*una parola è riservata in memoria per indicare quale device ha interrupts pendenti
  sulle linee da 3 a 7. Quindi se si tratta del terminale per esempio, è necessario spostarsi
  di 7 - 3 = 4 parole in avanti da PENDING_BITMAP_START. Successivamente è necessario fare uno shift
  a destra di dev_no posizioni per vedere se il device dev_no associato a line_no ha un interrupt pendente.
  Dopo lo shift si fa un & bitwise con 0x00000001 in modo da vedere se effettivamente il bit destinato a
  dev_no è 1. */

  if(*INTR_CURRENT_BITMAP(line_no) & (1 << dev_no))
    return 1;


  return 0;
}

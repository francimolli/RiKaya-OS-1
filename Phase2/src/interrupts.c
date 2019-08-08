#include "interrupts.h"

void Interrupt_Handler(){

  /*per capire quale (o quali) interrupt sta pendendo è necessario controllare
  ciascun bit del campo IP del registro CAUSE. Si utilizza la macro CAUSE_IP_GET
  per verificare se l'i-esimo bit di IP vale 1.*/

  U32 cause = 0;

  if(curr_proc != NULL)
    cause = curr_proc->p_s.cause;
  else{//caso in cui il processore sia in stato waiting

    old_area = (state_t *) INT_OLDAREA;
    cause = old_area->cause;
  }

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
     while(*semaddr <= 0){
       Verhogen(semaddr);
     }

     SET_IT(TIME_SLICE);//acknowledgement scrivendo un nuovo valore all'interno dell'interval timer

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
        dev_status = &(dev->status);

        /*il processo corrente richiede un'operazione di I/O su questo device con la syscall
        Do_IO, perciò esso deve essere bloccato fino a quando l'operazione di I/O non è terminata
        (infatti nella Do_IO viene fatta una Passeren).
        Innanzitutto è necesssario controllare che il device sia libero e non occupato
        (ovvero il relativo semaforo ha valore 0).
        Si fa, dunque, una P sul semaforo del device ì (la P viene fatta da Do_IO), e quando
        si solleva l'interrupt perchè l'operazione è stata completata, viene fatta la V, inoltre viene
        scritto l'ack nel registro del device in modo da concludere l'operazione e farne partire un'altra
        immediatamente*/

        if(*semDevices.disk[i].s_key <= 0){
          woken_proc = removeBlocked(semDevices.disk[i].s_key);
          (*semDevices.disk[i].s_key)++;
          ProcBlocked--;
          woken_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, woken_proc);
        }

        dev->command = DEV_C_ACK;//acknowledgement
        //attesa completamento richiesta di I/O
        while(dev->status != DEV_S_READY)
          ;

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.disk[i].s_key < 0){
          woken_proc = removeBlocked(semDevices.disk[i].s_key);
          (*semDevices.disk[i].s_key)++;
          IO_request(woken_proc->command, (U32 *) dev, 0);
        }
      }
    }
  }

  //Tape Interrupt
  if(CAUSE_IP_GET(cause, INT_TAPE)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_TAPE ha un interrupt pendente
      if(getDevice(INT_TAPE, i)){

        dev = (dtpreg_t *) (TAPE_START + (i * DEV_REG_SIZE_W));
        dev_status = &(dev->status);

        if(*semDevices.tape[i].s_key <= 0){
          woken_proc = removeBlocked(semDevices.tape[i].s_key);
          (*semDevices.tape[i].s_key)++;
          ProcBlocked--;
          woken_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, woken_proc);
        }

        dev->command = DEV_C_ACK;//acknowledgement
        //attesa completamento richiesta di I/O
        while(dev->status != DEV_S_READY)
          ;

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.tape[i].s_key < 0){
          woken_proc = removeBlocked(semDevices.tape[i].s_key);
          (*semDevices.tape[i].s_key)++;
          IO_request(woken_proc->command, (U32 *) dev, 0);
        }
      }
    }
  }

  //Network Interrupt
  if(CAUSE_IP_GET(cause, INT_UNUSED)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_UNUSED(network) ha un interrupt pendente
      if(getDevice(INT_UNUSED, i)){

        dev = (dtpreg_t *) (NETWORK_START + (i * DEV_REG_SIZE_W));
        dev_status = &(dev->status);

        if(*semDevices.network[i].s_key <= 0){
          woken_proc = removeBlocked(semDevices.network[i].s_key);
          (*semDevices.network[i].s_key)++;
          ProcBlocked--;
          woken_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, woken_proc);
        }

        dev->command = DEV_C_ACK;//acknowledgement
        //attesa completamento richiesta di I/O
        while(dev->status != DEV_S_READY)
          ;

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.network[i].s_key < 0){
          woken_proc = removeBlocked(semDevices.network[i].s_key);
          (*semDevices.network[i].s_key)++;
          IO_request(woken_proc->command, (U32 *) dev, 0);
        }
      }
    }
  }


  //Printer Interrupt
  if(CAUSE_IP_GET(cause, INT_PRINTER)){

    for(int i = 0; i < DEV_PER_INT; i++){

      //controllo se il device i di tipo INT_PRINTER ha un interrupt pendente
      if(getDevice(INT_PRINTER, i)){

        dev = (dtpreg_t *) (PRINTER_START + (i * DEV_REG_SIZE_W));
        dev_status = &(dev->status);

        if(*semDevices.printer[i].s_key <= 0){
          woken_proc = removeBlocked(semDevices.printer[i].s_key);
          (*semDevices.printer[i].s_key)++;
          ProcBlocked--;
          woken_proc->p_s.reg_v0 = dev->status;
          insertProcQ(&ready_queue_h, woken_proc);
        }

        dev->command = DEV_C_ACK;//acknowledgement
        //attesa completamento richiesta di I/O
        while(dev->status != DEV_S_READY)
          ;

        //parte immediatamente un'altra operazione (se c'è)
        if(*semDevices.printer[i].s_key < 0){
          woken_proc = removeBlocked(semDevices.printer[i].s_key);
          (*semDevices.printer[i].s_key)++;
          IO_request(woken_proc->command, (U32 *) dev, 0);
        }
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

          if(*semDevices.terminalR[i].s_key <= 0 &&
             (term->recv_status & STATUSMASK) != DEV_S_READY){

                dev_status = &(term->recv_status);
                //caso in cui il terminale i ha sollevato un interrupt solo per la ricezione
                woken_proc = removeBlocked(semDevices.terminalR[i].s_key);
                (*semDevices.terminalT[i].s_key)++;
                ProcBlocked--;
                woken_proc->p_s.reg_v0 = term->recv_status;
                insertProcQ(&ready_queue_h, woken_proc);

                term->recv_command = DEV_C_ACK;

                while((term->recv_status & STATUSMASK) != DEV_S_READY)
                  ;
          }

          //prossima operazione
          if(*semDevices.terminalR[i].s_key < 0){

            woken_proc = removeBlocked(semDevices.terminalR[i].s_key);
            (*semDevices.terminalR[i].s_key)++;
            //chiamata di IO sul terminale in ricezione
            IO_request(woken_proc->command,(U32 *) term, TRUE);
          }

          if(*semDevices.terminalT[i].s_key <= 0 &&
          (term->transm_status & STATUSMASK) != DEV_S_READY){

                dev_status = &(term->transm_status);
                //caso in cui il terminale i ha sollevato un interrupt solo per la trasmissione
                woken_proc = removeBlocked(semDevices.terminalT[i].s_key);
                (*semDevices.terminalT[i].s_key)++;
                ProcBlocked--;
                woken_proc->p_s.reg_v0 = term->transm_status;
                insertProcQ(&ready_queue_h, woken_proc);

                term->transm_command = DEV_C_ACK;

          //prossima operazione
          if(*semDevices.terminalT[i].s_key < 0){

            woken_proc = removeBlocked(semDevices.terminalT[i].s_key);
            (*semDevices.terminalT[i].s_key)++;
            //chiamata di IO sul terminale in trasmissione
            IO_request(woken_proc->command,(U32 *) term, FALSE);
          }
          }

        }

    }
  }
  scheduler();
}

int getDevice(int line_no, int dev_no){

  /*questa funzione ritorna 1 se il device i attaccato alla linea j solleva un interrupt.
  Per identificare il device i si sfrutta l'indirizzo di PENDING_BITMAP_START, ovvero
  l'indirizzo in memoria dove inizia la Interrupting Devices Bit Map*/

  /*una parola è riservata in memoria per indicare quale device ha interrupts pendenti
  sulle linee da 3 a 7. Quindi se si tratta del terminale per esempio, è necessario spostarsi
  di 7 - 3 = 4 parole in avanti da PENDING_BITMAP_START. Successivamente è necessario fare uno shift
  a destra di dev_no posizioni per vedere se il device dev_no associato a line_no ha un interruptpendente.
  Dopo lo shift si fa un & bitwise con 0x00000001 in modo da vedere se effettivamente il bit destinato a
  dev_no è 1. */

  /*se non va prova ad usare le macro in fondo al file const_rikaya
  if(((PENDING_BITMAP_START + line_no) >> dev_no) & 0x00000001)
    return 1;*/

  if(*INTR_CURRENT_BITMAP(line_no) & (1 << dev_no))
    return 1;

  return 0;
}

void IO_request(U32 command, U32 *reg, U32 term_command){

  int *semaddr;
  U32 offset = 0;

  /*è necessario ottenere un indice all'interno di semDevices, ovvero ottener il semaforo associato
  al device che richiede di fare I/O. Per fare ciò si considera che da DEV_REGS_START a TERM0ADDR sono
  presenti le locazioni di memoria dedicate ai devices quali disk, tape, network, printer. Se reg è superiore a
  TERM0ADDR, allora il semaforo è associato ad un terminale e il commando può esssere associato alla
  trasmissione o alla ricezione in base al contenuto della variabile term_command.*/

  if(reg >= (U32 *)TERM0ADDR){
    //calcolo offset all'interno dell'array di semafori per il terminale
    offset = (reg - (U32 *)TERM0ADDR)/DEV_REG_SIZE_W;

    //registro associato ad un terminale
    if(term_command){

      //richiesta di I/O su un terminale in ricezione, si scrive il comando nel campo recv_command
      dev_status = &((termreg_t *)reg)->recv_status;
      semaddr = semDevices.terminalR[offset].s_key;
      if(~*semaddr)
        ((termreg_t *)reg)->recv_command = command;
    }
    else{

      //richiesta di I/O su un terminale in trasmissione, si scrive il comando nel campo transm_command
      dev_status = &((termreg_t *)reg)->transm_status;
      semaddr = semDevices.terminalT[offset].s_key;
      if(~*semaddr){
        ((termreg_t *)reg)->transm_command = command;
      }
    }
  }
  else{
    /*per calcolare l'offset è necessario sapere a quale indirizzo di memoria iniziano i registri dedicati ad
    ogni diverso tipo di device tra disk, tape, network e printer. Per ottenere l'indirizzo di memoria in cui
    si trovano i registri associati ai disk, tape, network o printer si usa la macro DEV_ADDRESS(LINENO, DEVNO),
    utilizzando come DEVNO lo 0. Per esempio per ottenere l'indirizzo di memoria da cui parte la memorizzazione
    dei devices di tipo tape, si usa DEV_ADDRESS(INT_TAPE - INT_LOWEST, 0). Infine per ottenere l'offset
    relativo al device che ha il registro reg, si procede come per il terminale.*/

    if(reg >= (U32 *)DISK_START && reg < (U32 *)TAPE_START){
      offset = (reg - (U32 *)DISK_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.disk[offset].s_key;
    }
    else if(reg >= (U32 *)TAPE_START && reg < (U32 *)NETWORK_START){
      offset = (reg - (U32 *)TAPE_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.tape[offset].s_key;
    }
    else if(reg >= (U32 *)NETWORK_START && reg < (U32 *)PRINTER_START){
      offset = (reg - (U32 *)NETWORK_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.network[offset].s_key;
    }
    else{
      offset = (reg - (U32 *)PRINTER_START)/DEV_REG_SIZE_W;
      semaddr = semDevices.printer[offset].s_key;
    }

    //registro associato ad un disk, tape, network o printer
    dev_status = &((dtpreg_t *)reg)->status;
    if(~*semaddr)
      ((dtpreg_t *)reg)->command = command;

  }
  if(semaddr != NULL){

    if(curr_proc == woken_proc){

      Passeren(semaddr);
    }
    else{

      (*semaddr)--;
      ProcBlocked++;
      insertBlocked(semaddr, woken_proc);
      outProcQ(&ready_queue_h, woken_proc);
    }
  }
}

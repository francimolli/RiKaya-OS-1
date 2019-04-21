# PHASE 1.5

In questa fase sono state parzialmente implementate :
- **Inizializzazione** del Sistema
- **Scheduling** dei processi
- Gestione delle **SysCall**
- Gestione degli **Interrupt**

## Compilazione

Per compilare i file sorgente in modo che possano essere  
eseguiti dall'architettura emulata da *μMPS2*, bisogna  
spostarsi all'interno della directory src e lanciare il  
comando make :  
```
cd src
make
```
Verrano così generati i file oggetto che saranno automaticamente  
linkati fra loro.

#### Nota per l'esecuzione
Per avere una corretta esecuzione, è necessario disabilitare la  
spunta *Exceptions* presente nella barra delle *Stop Mask*,  
in fondo all'interfaccia grafica di μMPS2.  

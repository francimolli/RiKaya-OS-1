int prin_putchar(char c);
/* Questa funzione prende in input un carattere (della tabella ASCII), che viene passato dal main, in particolare dalla funzione 'sendtoprinter'. 
 * Una volta passato un carattere, è necessario acquisire lo stato della stampante per capire se sia possibile utilizzarla e dunque si legge l'intero presente nel campo STATUS, presente ad un particolare indirizzo di memoria ottenibile grazie a printer.
 * Se STATUS vale ST_READY(1), allora è possibile utilizzare la stampante e andare a settare il campo DATA0 con il valore del carattere da stampare; inoltre si imposta il campo COMMAND (ottenibile sempre grazie a printer) al valore di trasmissione.
 * Una volta completata la stampa viene impostato il comando di acknowledgement e la stampante può essere riutilizzata.  */
int prin_puts(char *str);
/* prin_puts prende in input una puntatore a caratteri e passa a prin_putchar un carattere alla volta della stringa. */

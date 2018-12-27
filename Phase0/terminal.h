int term_putchar(char c);
/* term_putchar riceve in input un carattere cha dovrà essere trasmesso in output. Per fare ciò è necessario che il terminale sia nello stato ST_READY. 
 * Se dunque la variabile stat, utilizzata per memorizzare il valore dello stato del terminale, vale 1, è possibile scrivere nel terminale un carattere, e impostare quindi il comando CMD_TRANSMIT.
 * Nel caso in cui si trasmetta un carattere è necessario tenere conto che il relativo registro utilizzato per questo scopo memorizza prima il transmit command e successivamente il carattere da trasmettere. Ciò implica che la rappresentazione in binario del carattere subisca uno shift verso sinistra di 8 posizioni.
 * Negli zeri messi dall' operazione di shift si andrà a mettere il valore in binario del relativo comando di trasmissione.
 * Una volta trasmesso il carattere viene settato come comando CMD_ACK.*/
int term_puts(char *str);
/* Questa funzione viene utilizzata per passare un carattere alla volta a term_putchar. */
int term_getchar(void);
/* term_getchar gestisce il terminale quando lo si vuole utilizzare per ricevere caratteri.
 * Se lo stato del terminale è ST_READY, allora è possibile utilizzarlo per ricevere caratteri. A differenza della trasmissione, non c'è un carattere da memorizzare e dunque si memorizza solamente il comando CMD_RECV nel registro utilizzato per la ricezione.
 * Una volta che la ricezione è conclusa il comando CMD_ACK è impostato. */

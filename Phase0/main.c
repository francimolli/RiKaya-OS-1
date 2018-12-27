#include "umps/libumps.h"
#include "umps/arch.h"
#include "terminal.h"
#include "printer.h"
#include "punchCard.h"

#define LINE_BUF_SIZE 64 /* lunghezza massima input */
#define CONVERTED_WORD_SIZE 10 /* lunghezza di ogni carattere tradotto in - e * */

static char buf[LINE_BUF_SIZE]; /* buffer di input */
static char word[CONVERTED_WORD_SIZE]; /* array per memorizzare l'output */
static char ins[2]; /* array per memorizzare risposta(s/n) */
static char enter[] = "\n------------\n\n";

unsigned int strlen (char *str); /* prende in input una stringa e ne ritorna la lunghezza */

char digitToChar (unsigned int n); /* dato un intero in input, ritorna il corrispondente valore nella tabella ASCII */

unsigned int exp10 (int e);

char* uinttostr (unsigned int num, char* str); /* in base alla lettera che si sta traducendo viene convertita la percentuale di completamento da intero a stringa */

int sendtoprinter(char* word); /* manda alla stampante la stringa da stampare, ritorna 1 se la stampa va a buon fine */

static void readline(char *buf, unsigned int count); /* legge una stringa in input dal terminale */

int request(); /* chiede all'utente se si vuole effettuare un ulteriore stampa */

static void getPrint(); /* gestisce la stampa corrente, essendo possibili più stampe */

static void halt(void); /* spegne la macchina */

int main(int argc, char *argv[])
{
    unsigned int x = 1;
    
    while(x){ 
        getPrint();
    	x = request();
    }
     
    term_puts("\nPRINT COMPLETED!\n");
    
    halt();

    return 0;
}

unsigned int strlen (char *str) {

    unsigned int len = 0;

    while(*(str++) != '\0' && ++len) ;

    return len;
}

char digitToChar (unsigned int n) {
    
    char c = '0';
    
    return c + n;
}

unsigned int exp10 (int e) {
    
    unsigned int value = 1;
    
    while (e-- > 0) 
	    value *= 10;
    
    return value;
}

char* uinttostr (unsigned int num, char* str) {
    /* funzione necessaria per convertire un intero in una stringa del momento che il terminale può ricevere solo stringhe e non interi */
    char* firstelement = str;
    int e = 1; /* variabile per individuare il numero di cifre del completamento */
    
    while (num >=  exp10(e)) 
        e++;
   
    str += e; /* si avanza nell'array per la stampa della percentuale di completamento di un numero di cifre (posizioni) pari ad e*/
    *(str) = '\0';
    str--;
    
    while (e > 0) {
        /* si inseriscono le cifre della percentuale nell'array a ritroso, in modo da inserire la cifra meno significativa per prima */
        *str = digitToChar ( num % 10);
        str--;    
        num /= 10;
        e--;
    }

    return firstelement;
}

int sendtoprinter(char* word) {
      
    return prin_puts(word);
}

static void readline(char *p, unsigned int count) {
    
    int c;

    while (--count && (c = term_getchar()) != '\n')
        *p++ = c;

    *p = '\0';
}



static void getPrint() {

    char *tmpbuf = buf;
    
    term_puts("Insert what you want to print : \n");
    readline(buf, LINE_BUF_SIZE);
    term_puts("Started to print ...\n \n");
  
    unsigned int len = strlen(buf); /* lunghezza input letto */
    char tmpstr[20]; /* array utilizzato per stampare la percentuale di completamento */
    int error = 0;
    unsigned int counter = 1; /* numero carattere dell'input che si sta traducendo */

    while (tmpbuf && *tmpbuf != '\0') {
        if(sendtoprinter(cStrToPunch(tmpbuf,word))) { /* cStrToPunch tarduce il carattere puntato da tmpbuf in sequenze di - e * */
            term_puts("printing - ");
            term_puts(uinttostr((counter*100)/len,tmpstr)); /* stampa nel terminale della percentuale di completamento */
            term_puts("% ...\n");
        }
        else error = 1;
        
        tmpbuf++;
        counter++;
    }
    
    if (!error) term_puts("\nPRINT COMPLETED!\n \n");
}

int request() {
    char s[1] = "s";
    char t[1] = "S";

    term_puts("May need another print ? (S/n)\n");
    readline(ins, CONVERTED_WORD_SIZE);
    term_puts("\n");    

    if(*ins == *s || *ins == *t) return(sendtoprinter(enter));
    else return 0;
}

static void halt(void) {
    
    WAIT();
    
    *((volatile unsigned int *) MCTL_POWER) = 0x0FF;
    
    while(1);
}

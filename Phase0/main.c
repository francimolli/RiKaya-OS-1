#include "umps/libumps.h"
#include "umps/arch.h"
#include "terminal.h"
#include "printer.h"
#include "punchCard.h"

#define LINE_BUF_SIZE 64
#define CONVERTED_WORD_SIZE 10

static char buf[LINE_BUF_SIZE];
static char word[CONVERTED_WORD_SIZE];


unsigned int strlen (char *str) {

    unsigned int len = 0;

    while(*(str++) != '\0' && ++len) ;

    return len;

}
/*
char* strcat(char c, char *str){

    int len = strlen(str);
    len--;
    for(; len >=  0; len--)
	    str[len + 4] = str[len];
    str[0] = c;
    str[1] = ' ';
    str[2] = '=';
    str[3] = ' ';
    
    return str;
}
*/
char digitToChar (unsigned int n) {
    
    char c = '0';
    return c + n;

}

unsigned int exp10 (int e) {

    unsigned int value = 1;
    while (e-- > 0) value *= 10;
    return value;

}

char* uinttostr (unsigned int num, char* str) {
   
    char* firstelement = str;

    int e = 1;
    
    
    while (num >= exp10(e)) 
        e++;
    
    
    str += e;
    *(str) = '\0';
    str--;
    
    while (e > 0) {
        
        *str = digitToChar ( num % 10);
        str--;
        
        num /= 10;
        e--;


    }

    return firstelement;

}

int sendtoprinter(char* word)
{
      return prin_puts(word);
}

static void readline(char *buf, unsigned int count)
{
    int c;

    while (--count && (c = term_getchar()) != '\n')
        *buf++ = c;

    *buf = '\0';
}


static void halt(void)
{
    WAIT();
    *((volatile unsigned int *) MCTL_POWER) = 0x0FF;
    while (1) ;
}

int main(int argc, char *argv[])
{
	
    char *tmpbuf = buf;
    term_puts("Insert what you want to print : \n");

    readline(buf, LINE_BUF_SIZE);
    term_puts("Started to print ...\n \n");
    
        
    unsigned int len = strlen(buf);
    

    char tmpstr[20];
    
    int error = 0;

    unsigned int counter = 1;
    while (tmpbuf && *tmpbuf != '\0') {
        if(sendtoprinter(cStrToPunch(tmpbuf,word))) {
            term_puts("printing - ");
            term_puts(uinttostr((counter*100)/len,tmpstr));
            term_puts("% ...\n");
        }
        else error = 1;
        
        tmpbuf++;
        counter++;
    }
    
    if (!error) term_puts("\nPRINT COMPLETED!\n");
    

    halt();

    return 0;
}

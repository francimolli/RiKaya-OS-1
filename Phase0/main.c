#include "umps/libumps.h"
#include "umps/arch.h"
#include "terminal.h"
#include "printer.h"

#define LINE_BUF_SIZE 64

static char buf[LINE_BUF_SIZE];

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

    term_puts("Insert what you want to print : \n");

    readline(buf, LINE_BUF_SIZE);
    term_puts("Started to print ...\n \n");
    
    if(sendtoprinter(buf)) term_puts("Print complete!\n");

    halt();

    return 0;
}

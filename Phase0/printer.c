#include "umps/arch.h"
#include "umps/types.h"

#define ST_NOTINSTALLED    0
#define ST_READY           1
#define ST_UNKNOWNCOMMAND  2
#define ST_BUSY            3
#define ST_PRINTERROR      4

#define CMD_RESET          0
#define CMD_ACK            1
#define CMD_TRANSMIT       2

volatile dtpreg_t *printer = (dtpreg_t*) DEV_REG_ADDR(IL_PRINTER, 0);

static unsigned int printer_status(volatile dtpreg_t *prin);

int prin_putchar(char c)
{
    unsigned int stat;

    stat = printer_status(printer);
    if (stat != ST_READY && stat == ST_PRINTERROR)
        return 0;

    printer->data0 = c;
    printer->command = CMD_TRANSMIT;

    while ((stat = printer_status(printer)) == ST_BUSY)
        ;

    if (stat != ST_READY)
        return 0;

    printer->command = CMD_ACK;

    return 1;
}

int prin_puts(char *str)
{
    for (; *str; ++str)
        if (!prin_putchar(*str))
            return 0;
    return 1;
}


static unsigned int printer_status(volatile dtpreg_t *prin)
{
    return (prin->status);
}


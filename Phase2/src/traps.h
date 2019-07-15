#ifndef TRAPS_H
#define TRAPS_H

#include <umps/libumps.h>
#include "scheduler.h"
#include "syscalls.h"
#include "utils.h"

void SysBpTrapHandler();
void TlbTrapHandler();
void PgmTrapHandler();

#endif

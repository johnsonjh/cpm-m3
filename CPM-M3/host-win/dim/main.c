#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "bios.h"
#include "modubios.h"
#include "diverge.h"

unsigned int cpm_bdos( unsigned int func, unsigned int parm );

int main( void )
{
  bios_cboot();
}


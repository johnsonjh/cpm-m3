#include "config.h"
#include "cpm.h"
#include "bios.h"
#include "modubios.h"
#include "platform.h"

/***/

bios_mrt_t platform_mrt = {
  1,
  { { (void *)0x04002000, 0x04014000 - 0x04002000 } }
};

/***/

extern modubios_cons_t consnull;
extern modubios_cons_t conswin;

modubios_cons_t *platform_consoles[ 16 ] = {
  &conswin, &consnull, &consnull, &consnull,
  &consnull, &consnull, &consnull, &consnull,
  &consnull, &consnull, &consnull, &consnull,
  &consnull, &consnull, &consnull, &consnull
};

/***/

extern modubios_disk_t disknull;
extern modubios_disk_t diskrom;

modubios_disk_t *platform_disks[ 16 ] = {
  &diskrom, &disknull, &disknull, &disknull,
  &disknull, &disknull, &disknull, &disknull,
  &disknull, &disknull, &disknull, &disknull,
  &disknull, &disknull, &disknull, &disknull
};



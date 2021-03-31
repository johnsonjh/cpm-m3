call compiledri ..\..\target\os\dri-68k\bdosmain.c
call compiledri ..\..\target\os\dri-68k\bdosmisc.c
call compiledri ..\..\target\os\dri-68k\bdosrw.c
call compiledri ..\..\target\os\dri-68k\ccp.c
call compiledri ..\..\target\os\dri-68k\conbdos.c
call compiledri ..\..\target\os\dri-68k\dskutil.c
call compiledri ..\..\target\os\dri-68k\fileio.c
call compiledri ..\..\target\os\dri-68k\iosys.c

call compilenondri ..\..\target\os\glue\glue.c
call compilenondri ..\..\target\os\glue\calledbydri.c
call compilenondri ..\..\target\os\glue\modubios.c
call compilenondri ..\..\target\os\glue\consnull.c
call compilenondri ..\..\target\os\glue\disknull.c

call compilenondri arch-host.c
call compilenondri platform.c
call compilehost main.c
call compilehost conswin.c
call compilehost diskrom.c
call compilehost ccphooks.c
call compilehost exchange.c

cl /Fedim.exe /Fmdim.map *.obj

copy/y dim.exe ..\..\buildimg

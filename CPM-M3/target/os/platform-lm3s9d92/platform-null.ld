SEARCH_DIR( "/program files/codesourcery/sourcery_codebench_lite_for_arm_eabi/lib/gcc/arm-none-eabi/4.7.3" )
SECTIONS 
{
  locore 0xc0700000 : {
    platform.o(.text)
    platform.o(.rodata)
    platform.o(.data)
    platform.o(COMMON)
    platform.o(.bss)
  }
  .text 0xc7e00000 : AT( LOADADDR( locore ) + SIZEOF( locore ) ) {
    *(.text)
    *(.rodata)
  } 
  .data : ALIGN( 4 ) { 
    *(.data) 
    *(.image)
  }
  .bss  : ALIGN( 4 ) { 
    *(.bss) 
    *(COMMON)
  }

  locore_copy_from = LOADADDR( .text );
  locore_copy_to = ADDR( .text );
  locore_copy_size = LOADADDR( .bss ) - LOADADDR( .text );

  locore_tpa_start = LOADADDR( locore );
  locore_tpa_size =  ADDR( .text ) - LOADADDR( locore );

  locore_bss_start = ADDR( .bss );
  locore_bss_end = ADDR( .bss ) + SIZEOF( .bss );
}

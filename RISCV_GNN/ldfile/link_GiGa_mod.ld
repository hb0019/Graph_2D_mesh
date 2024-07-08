OUTPUT_ARCH( "riscv" )
MEMORY
{
MEM2(RWX)  : ORIGIN = 0x2000C000,  LENGTH = 0x8000
}
ENTRY(_start)

SECTIONS {
    .text :
    {
        *(.text)
        *(.text.init*)
        . = ALIGN(0x10);
    } >MEM2
    .text :
    {
        *(.text*)
        . = ALIGN(0x10);
    } >MEM2
    .rodata :
    {
      *(.rodata*)
      . = ALIGN(0x4);
        __erodata = .;
    } > MEM2
    .data :
    {
      . = ALIGN(0x4);
      __data_start__ = .;
        *(.data*)
          . = ALIGN(0x4);
        __data_end__ = .;
                _end = .;
                 end = . ;
        PROVIDE( _end = .);
        PROVIDE ( end = . );
      STACK = ALIGN(16) + 0x1000;
    } >MEM2
    .bss :
    {
      . = ALIGN(0x4);
      __bss_start__ = .;
        *(.bss)
         . = ALIGN(0x4);
        __bss_end__ = .;
 	 __end = . ;
        end = . ;
          *.(COMMON)
    } >MEM2
}

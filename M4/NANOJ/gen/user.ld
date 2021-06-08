/* Linker script for Nanotec VMM V2.00
 *
 * VMM header is loaded to Flash at 0x08008000
 * VMM Code starts at 0x08008200, size is 15.5k
 *
 * Mapping area is identical with V1.00 VMM
 * Stack is same size (320 Bytes), heap is 3.5k
 */

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
ENTRY(start)
SEARCH_DIR(.)

MEMORY
{
  rom (rx)  : ORIGIN = 0x08008200, LENGTH = 15872
  input(rwx)  : ORIGIN = 0x20000E00, LENGTH = 64
  output (rwx)  : ORIGIN = 0x20000E40, LENGTH = 64
  inoutput(rwx)  : ORIGIN = 0x20000E80, LENGTH = 64
  ram (rwx)  : ORIGIN = 0x20000000, LENGTH = 3584
}

EXTERN(_start)

SECTIONS {
  .text : {
  *(.boot)
  *(.text)
  } > rom 
  .inputs : {  
   *(.inputs)
   } > input
  .outputs : {  
   *(.outputs)
   } > output  
  .inoutputs : {  
   *(.inoutputs)
   } > inoutput  
  .data : {  
   *(.data) 
   } > ram
  .rodata : {  
   *(.data) 
   } > rom 
  .bss :  { *(.bss)  *(COMMON) } > ram
}

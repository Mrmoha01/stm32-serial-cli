/* Linker script for the STM32C031C6: 32 KB flash, 12 KB RAM. */

ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 12K
}

/* The stack starts at the top of RAM and grows downwards. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  /* The vector table must be at the very start of flash. */
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  /* Code and read-only data stay in flash. */
  .text :
  {
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .ARM.exidx :
  {
    *(.ARM.exidx*)
  } > FLASH

  /* Initial values of global variables are stored in flash ... */
  _sidata = LOADADDR(.data);

  /* ... and copied to RAM by Reset_Handler. */
  .data :
  {
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT > FLASH

  /* Zero-initialised globals live only in RAM. */
  .bss (NOLOAD) :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  PROVIDE(end = .);
  PROVIDE(_end = .);
}

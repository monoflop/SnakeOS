/*
 * Thanks to http://www.lowlevel.eu for the implementation
 */
/* Entrypoint */
ENTRY(_start)

/*
 * Binary sections
 */
SECTIONS
{
    /*
     * Kernel should be loaded to 0x100000
     * kernel_start and kernel_end is defined so we can check the kernel size
     */
    . = 0x100000;
	kernel_start = .;

  /*
   * Multiboot header have to be in the first 6Kb of the binary
   */
    .text : {
        *(multiboot)
        *(.text)
    }
    .data ALIGN(4096) : {
        *(.data)
    }
    .rodata ALIGN(4096) : {
        *(.rodata)
    }
    .bss ALIGN(4096) : {
        *(.bss)
    }
	. = ALIGN(4096);
    kernel_end = .;
}

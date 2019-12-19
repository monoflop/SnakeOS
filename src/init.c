#include "stdio.h"
#include "serial.h"
#include "interrupt.h"
#include "multiboot.h"
#include "mm.h"
#include "snake.h"

void init(struct multiboot_info *mb_info)
{
	//Initialise Serial Communication
	//Because we dont use 80x25 text mode in the snake game,
	//we write everything to COM1
	#ifdef KERNEL_COM_OUTPUT
		init_serial();
	#endif

	cls();
	printf("---------------------------SnakeOS by Philipp Kutsch-----------------------\n");
	printf("  /~\\    _______  _____  __   _  _____  _______         _____   _____  \n");
	printf(" C oo   |  |  |  |     | | \\  | |     | |______ |      |     | |_____] \n");
	printf(" _( ^)  |  |  |  |_____| |  \\_| |_____| |       |_____ |_____| |       \n");
	printf("/   ~\\ \n");
	printf("---------------------------------------------------------------------------\n");

	//Initialise physical memory management
	//Physical memory is split into pages of 4096 Bytes.
	//At the moment if we need to allocate memory in the game we always allocate
	//pages of 4096 Bytes.
	//@TODO Implement propper memory management
	init_mm(mb_info);

	//Setup keyboard
	//Clear keyboard buffer
	while (inportb(0x64) & 0x1)
	{
		inportb(0x60);
	}

	//Activate keyboard
	while ((inportb(0x64) & 0x2)) {}
	outportb(0x60, 0xF4);

	//Setup Global Descriptor Table
	init_gdt();

	//Setup Interrupts
	init_intr();

	//Start snake game
	snake_init();

	//We should never return from the init method
	while(1){}
}

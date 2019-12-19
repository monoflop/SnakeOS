/*
 * Thanks to http://www.lowlevel.eu for parts of the implementation
 */
#include "interrupt.h"

#include "stdint.h"
#include "cpu.h"
#include "stdio.h"
#include "snake.h"
#include "bios_int.h"

extern void intr_stub_0(void);
extern void intr_stub_1(void);
extern void intr_stub_2(void);
extern void intr_stub_3(void);
extern void intr_stub_4(void);
extern void intr_stub_5(void);
extern void intr_stub_6(void);
extern void intr_stub_7(void);
extern void intr_stub_8(void);
extern void intr_stub_9(void);
extern void intr_stub_10(void);
extern void intr_stub_11(void);
extern void intr_stub_12(void);
extern void intr_stub_13(void);
extern void intr_stub_14(void);
extern void intr_stub_15(void);
extern void intr_stub_16(void);
extern void intr_stub_17(void);
extern void intr_stub_18(void);

extern void intr_stub_32(void);
extern void intr_stub_33(void);

//GDT and IDT
static uint64_t gdt[GDT_ENTRIES];
static uint64_t idt[IDT_ENTRIES];

//TSS
static uint32_t tss[32] = { 0, 0, 0x10 };

static void gdt_set_entry(int i, unsigned int base, unsigned int limit, int flags)
{
  gdt[i] = limit & 0xffffLL;
  gdt[i] |= (base & 0xffffffLL) << 16;
  gdt[i] |= (flags & 0xffLL) << 40;
  gdt[i] |= ((limit >> 16) & 0xfLL) << 48;
  gdt[i] |= ((flags >> 8 )& 0xffLL) << 52;
  gdt[i] |= ((base >> 24) & 0xffLL) << 56;
}

void init_gdt(void)
{
  //required null descriptor
  gdt_set_entry(0, 0, 0, 0);

  //kernel code segment descriptor
  gdt_set_entry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);

  //kernel data segment descriptor
  gdt_set_entry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);

  //ring level 3 code segement descriptor
  gdt_set_entry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

  //ring level 3 segment descriptor
  gdt_set_entry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

  //TSS segment descriptor
	gdt_set_entry(5, (uint32_t) tss, sizeof(tss),
    GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

  //pack gdt
  struct {
  	    uint16_t limit;
  	    void* pointer;
  } __attribute__((packed)) gdtp = {
  	    .limit = GDT_ENTRIES * 8 - 1,
   	   .pointer = gdt,
  };

  //instruct cpu to load gdt at gdtp pointer
	__asm__ volatile("lgdt %0" : : "m" (gdtp));

	__asm__ volatile(
        "mov $0x10, %ax;"
        "mov %ax, %ds;"
        "mov %ax, %es;"
        "mov %ax, %ss;"
        "ljmp $0x8, $.1;"
        ".1:"
    );

  //load task register
	__asm__ volatile("ltr %%ax" : : "a" (5 << 3));
}

static void idt_set_entry(int i, void (*fn)(), unsigned int selector, int flags)
{
  unsigned long int handler = (unsigned long int) fn;
  idt[i] = handler & 0xffffLL;
  idt[i] |= (selector & 0xffffLL) << 16;
  idt[i] |= (flags & 0xffLL) << 40;
  idt[i] |= ((handler>> 16) & 0xffffLL) << 48;
}

static void init_pic(void)
{
  // Our interrupts are mapped
  // Standard ISA IRQs
  // IRQ 0 -> 0x20 (PIT)
  // IRQ 1 -> 0x21 (Keyboard)
  // ...

  // Master
  outportb(0x20, 0x11);
  outportb(0x21, 0x20);
  outportb(0x21, 0x04);
  outportb(0x21, 0x01);

  // Slave
  outportb(0xa0, 0x11);
  outportb(0xa1, 0x28);
  outportb(0xa1, 0x02);
  outportb(0xa1, 0x01);

  // Active all interrupts
  outportb(0x20, 0x0);
  outportb(0xa0, 0x0);
}

void init_intr(void)
{
    // Pack IDT
    struct {
        unsigned short int limit;
        void* pointer;
    } __attribute__((packed)) idtp = {
        .limit = IDT_ENTRIES * 8 - 1,
        .pointer = idt,
    };

    // Initialise PIC
    init_pic();

    // Excpetions
    idt_set_entry(0, intr_stub_0, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(1, intr_stub_1, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(2, intr_stub_2, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(3, intr_stub_3, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(4, intr_stub_4, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(5, intr_stub_5, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(6, intr_stub_6, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(7, intr_stub_7, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(8, intr_stub_8, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(9, intr_stub_9, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(10, intr_stub_10, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(11, intr_stub_11, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(12, intr_stub_12, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(13, intr_stub_13, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(14, intr_stub_14, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(15, intr_stub_15, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(16, intr_stub_16, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(17, intr_stub_17, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(18, intr_stub_18, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

    // IRQs
    idt_set_entry(32, intr_stub_32, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    idt_set_entry(33, intr_stub_33, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

    __asm__ volatile("lidt %0" : : "m" (idtp));

    __asm__ volatile("sti");
}

// Interrupt service routine (ISR)
struct cpu_state* handle_interrupt(struct cpu_state* cpu)
{
  struct cpu_state* new_cpu = cpu;

	if (cpu->intr <= 0x1f)
	{
		cls();
    printf("Exception %d, kernel stopped!\n", cpu->intr);

		printf("----------------------------------------------\n");
		printf("I'm the black? screen of DEATH :)\n");
		printf("----------------------------------------------\n");

		printf("eax=0x%x\n", cpu->eax); //Accumulator register
    printf("ebx=0x%x\n", cpu->ebx); //Base register
    printf("ecx=0x%x\n", cpu->ecx); //Counter register
    printf("edx=0x%x\n", cpu->edx); //Data register

    printf("esi=0x%x\n", cpu->esi); //Source index register
    printf("edi=0x%x\n", cpu->edi); //Destination index register
    printf("ebp=0x%x\n", cpu->ebp); //Stack Base pointer register

    printf("intr=0x%x\n", cpu->intr); //Interrupt number
    printf("error=0x%x\n", cpu->error); //Error number

    printf("eip=0x%x\n", cpu->eip); //Index Pointer
    printf("cs=0x%x\n", cpu->cs); //Code segment
    printf("eflags=0x%x\n", cpu->eflags); //Processor state
    printf("esp=0x%x\n", cpu->esp); //Stack pointer register
    printf("ss=0x%x\n", cpu->ss); //Stack segment

		printf("----------------------------------------------\n");

    while(1)
		{
      //hold cpu
      __asm__ volatile("cli; hlt");
    }
  }
  else if (cpu->intr >= 0x20 && cpu->intr <= 0x2f)
	{
		//PIT interrupt
		if (cpu->intr == 0x20)
		{
      //We use the PIT interrupt for crude ingame timing
			on_tick();
		}

		//Keyboard interrupt
		if(cpu->intr == 0x21)
		{
      //Simple ingame input
			uint8_t scancode = inportb(0x60);
      on_key(scancode);
		}
    if (cpu->intr >= 0x28)
		{
      //Send EOI to Slave-PIC
      outportb(0xa0, 0x20);
    }

    //Send EOI to Master-PIC
    outportb(0x20, 0x20);
  }
	else
	{
    printf("Unknown interrupt\n");
    while(1)
		{
      //hold cpu
      __asm__ volatile("cli; hlt");
    }
  }
	return new_cpu;
}

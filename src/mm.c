/*
 * Thanks to http://www.lowlevel.eu for parts of the implementation
 */
#include "mm.h"

#include "stdio.h"
#include "multiboot.h"

#define MM_PAGE_USED 0
#define MM_PAGE_FREE 1
#define BITMAP_SIZE 32768
#define PAGE_SIZE 4096

extern const void kernel_start;
extern const void kernel_end;

static uint32_t bitmap[BITMAP_SIZE];

void init_mm(struct multiboot_info *mb_info)
{
	//Lower and upper memory fields are only valid if the coresponding flag is non null
	//@TODO
	printf("mem_lower=%d\n", mb_info->mem_lower);
	printf("mem_upper=%d\n", mb_info->mem_upper);

	//Mark everything as used
	memset(bitmap, MM_PAGE_USED, BITMAP_SIZE);

	//Apply memory map from multiboot
	struct multiboot_mmap_entry* mmap_entry = (void*)mb_info->mmap_addr;
	for(int i = 0; i < mb_info->mmap_length; i++)
	{
		if(mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE)
		{
			//Free memory:
			printf("Freeing %d KB at ", (mmap_entry->len / 1024));//@TODO fix printf....
			printf("0x%p\n", mmap_entry->addr);

			uintptr_t addr = mmap_entry->addr;
	    uintptr_t end_addr = addr + mmap_entry->len;
	    while (addr < end_addr)
			{
	     	mm_free((void*) addr);
	     	addr += 0x1000;
	    }
		}

		mmap_entry++;
	}

	//Mark multiboot structure
	//Structure is probably not larger than 4k
	mm_mark_used((void *) mb_info);

	//Mark multiboot modules as used.
	mm_mark_used((void*)mb_info->mods_addr);
	struct multiboot_mod_list* multiboot_mod = (void*)mb_info->mods_addr;
	printf("Found %d multiboot modules\n", mb_info->mods_count);
	for(int i = 0; i < mb_info->mods_count; i++)
	{
		printf("mod start 0x%p\n", multiboot_mod->mod_start);
		printf("mod end 0x%p\n", multiboot_mod->mod_end);
		uintptr_t addr = multiboot_mod->mod_start;
		uintptr_t end_addr = multiboot_mod->mod_end;
		while (addr < end_addr)
		{
			mm_mark_used((void *) addr);
			addr += 0x1000;
		}

		multiboot_mod++;
	}

	//Mark kernel as used.
	uintptr_t addr = (uintptr_t) &kernel_start;
	printf("Kernel start 0x%p\n", addr);
	printf("Kernel end 0x%p\n", (uintptr_t) &kernel_end);
	while (addr < (uintptr_t) &kernel_end)
	{
    mm_mark_used((void*) addr);
  	addr += 0x1000;
	}

	//Mark the first page as used
	mm_mark_used((void*)0x0);

	//@TODO Detect Upper Memory with BIOS INT 0x15, EAX = 0xE820
	//Alloc 2MB because writing on memory this low causes glitches
	//on real mashines
	for(int i = 0; i < 512; i++)mm_alloc();
}

void * mm_alloc()
{
	for(int i = 0; i < BITMAP_SIZE; i++)
	{
		for(int bit_index = 0; bit_index < 32; bit_index++)
		{
			if(bitmap[i] & (1 << bit_index))
			{
				bitmap[i] &= ~(1UL << bit_index);
				return (void*)(i * 131072 + bit_index * PAGE_SIZE);
			}
		}
	}

	return NULL;
}

void mm_free(void * addr)
{
	uintptr_t address = (uintptr_t)addr;
	if(address % PAGE_SIZE != 0)return;

	//calculate index position:
	int index = address / PAGE_SIZE / 32;
	int bit = (address / PAGE_SIZE) % 32;

	if(!CHECK_BIT(bitmap[index], bit))
		bitmap[index] |= 1UL << bit;
}

void mm_mark_used(void * addr)
{
	uintptr_t address = (uintptr_t)addr;
	if(address % PAGE_SIZE != 0)
	{
		printf("address is not multiple of the page size\n");
		return;
	}

	//calculate index position:
	int index = address / PAGE_SIZE / 32;
	int bit = (address / PAGE_SIZE) % 32;

	if(CHECK_BIT(bitmap[index], bit))
		bitmap[index] &= ~(1UL << bit);
}

#ifndef MM_H
#define MM_H

#include "multiboot.h"

void init_mm(struct multiboot_info *mb_info);
void* mm_alloc();
void mm_free(void* addr);
void mm_mark_used(void * addr);
#endif

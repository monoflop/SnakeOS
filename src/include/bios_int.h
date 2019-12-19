/*
; Protected Mode BIOS Call Functionailty v2.0 - by Napalm
; -------------------------------------------------------
;
; This is code shows how its POSSIBLE to execute BIOS interrupts
; by switch out to real-mode and then back into protected mode.
;
; If you wish to use all or part of this code you must agree
; to the license at the following URL.
;
; License: http://creativecommons.org/licenses/by-sa/2.0/uk/
;
*/

#ifndef BIOS_INT_H
#define BIOS_INT_H

typedef struct __attribute__ ((packed))
{
	unsigned short di, si, bp, sp, bx, dx, cx, ax;
	unsigned short gs, fs, es, ds, eflags;
} regs16_t;

extern void int32(unsigned char intnum, regs16_t *regs);

#endif

#include "intrman.h"

#include "common.h"
#include "excepman.h"
#include "intrman_internal.h"
#include "introld.h"
#include "kerror.h"
#include "loadcore.h"
#include "sys/types.h"

#include <string.h>

#define I_CFG ((volatile u_int *)0xBF801450)
#define I_STAT ((volatile u_int *)0xBF801070)
#define I_MASK ((volatile u_int *)0xBF801074)
#define I_CTRL ((volatile u_int *)0xBF801078)
#define D_DICR ((volatile u_int *)0xBF8010F4)
#define D_DICR2 ((volatile u_int *)0xBF801574)

#define HANDLER_INFO(x) ((u_int *)(0x480 + ((x) * 8)))
#define HANDLER_OPT(x) ((u_int *)(0x484 + ((x) * 8)))

struct intr_handler {
	u_int info;
	void *opt;
};

struct intrman {
	int unk0;
	int unk4;
	int unk8;
	int unkc;
};

char intr_stack[0x800];

struct intrman im;

void exc_irq();
void exc_irq_fb();
void exc_syscall();
int dma_irq();

int func_000003DC();
void func_000003F0();
int func_00000418();

extern libhead intrman_entry;

int
start()
{
	int prid;
	int i;

	asm volatile("mfc0 %0, $15  \n"
				 "nop           \n"
	  : "=r"(prid));

	if (prid < 16) {
		return NO_RESIDENT_END;
	}

	if ((*I_CFG & 8)) {
		return NO_RESIDENT_END;
	}

	*I_MASK = 0;
	*D_DICR = 0;
	*D_DICR2 = 0;

	for (i = 0; i < 0x80; i++) {
		((u_int *)0x480)[i] = 0;
	}

	*((u_int *)0x41c) = -1;
	im.unk4 = -1;
	im.unk8 = -1;

	im.unk0 = 0x480;

	RegisterExceptionHandler(EXCEP_Int, exc_irq);
	RegisterPriorityExceptionHandler(EXCEP_Int, 3, exc_irq_fb);
	RegisterExceptionHandler(EXCEP_Sys, exc_syscall);
	RegisterIntrHandler(INUM_DMA, HTYPE_C, dma_irq, NULL);
	RegisterLibraryEntries(&intrman_entry);

	return RESIDENT_END;
}

int
func_00000104()
{
	return 0;
}

int
func_0000010C()
{
	*I_MASK = 0;
	*D_DICR = 0;
	*D_DICR2 = 0;
	return 0;
}

struct intrman *
func_00000138()
{
	return &im;
}

int
RegisterIntrHandler(int intrcode, int type, intr_handler func, void *common)
{
	int oldstat;
	int ret;

	ret = KE_OK;

	if (QueryIntrContext()) {
		return KE_ILLEGAL_CONTEXT;
	}

	CpuSuspendIntr(&oldstat);

	if (intrcode < 0) {
		ret = KE_ILLEGAL_INTRCODE;
	} else if (intrcode < INUM_DMA_0) {
		if (!*HANDLER_INFO(intrcode)) {
			*HANDLER_INFO(intrcode) = (u_int)func | type & 3;
			*HANDLER_OPT(intrcode) = (u_int)common;
		} else {
			ret = KE_FOUND_HANDLER;
		}
	} else if (intrcode < INUM_DMA_7) {
		if (!*HANDLER_INFO(intrcode)) {
			*HANDLER_INFO(intrcode) = (u_int)func;
			*HANDLER_OPT(intrcode) = (u_int)common;
		} else {
			ret = KE_FOUND_HANDLER;
		}
	} else if (intrcode <= INUM_DMA_12) {
		if (!*HANDLER_INFO(intrcode)) {
			*HANDLER_INFO(intrcode) = (u_int)func;
			*HANDLER_OPT(intrcode) = (u_int)common;
		} else {
			ret = KE_FOUND_HANDLER;
		}
	} else if (intrcode == INUM_SW1 || intrcode == INUM_SW2) {
		if (!*HANDLER_INFO(intrcode - 16)) {
			*HANDLER_INFO(intrcode - 16) = (u_int)func | type & 3;
			*HANDLER_OPT(intrcode - 16) = (u_int)common;
		} else {
			ret = KE_FOUND_HANDLER;
		}
	} else {
		ret = KE_ILLEGAL_INTRCODE;
	}

	CpuResumeIntr(oldstat);
	return ret;
}

int
ReleaseIntrHandler(int intrcode)
{
	int oldstat;
	int ret;

	ret = KE_OK;

	if (QueryIntrContext()) {
		return KE_ILLEGAL_CONTEXT;
	}

	CpuSuspendIntr(&oldstat);

	if (intrcode < 0) {
		ret = KE_ILLEGAL_INTRCODE;
	} else if (intrcode < INUM_DMA_0) {
		if (*HANDLER_INFO(intrcode)) {
			*HANDLER_INFO(intrcode) = 0;
		} else {
			ret = KE_NOTFOUND_HANDLER;
		}
	} else if (intrcode < INUM_DMA_7) {
		if (*HANDLER_INFO(intrcode)) {
			*HANDLER_INFO(intrcode) = 0;
		} else {
			ret = KE_NOTFOUND_HANDLER;
		}
	} else if (intrcode <= INUM_DMA_12) {
		if (*HANDLER_INFO(intrcode)) {
			*HANDLER_INFO(intrcode) = 0;
		} else {
			ret = KE_NOTFOUND_HANDLER;
		}
	} else if (intrcode == INUM_SW1 || intrcode == INUM_SW2) {
		if (*HANDLER_INFO(intrcode - 16)) {
			*HANDLER_INFO(intrcode - 16) = 0;
		} else {
			ret = KE_NOTFOUND_HANDLER;
		}
	} else {
		ret = KE_ILLEGAL_INTRCODE;
	}

	CpuResumeIntr(oldstat);
	return ret;
}

int
CpuSuspendIntr(int *oldstat)
{
	int stat;

	stat = *I_CTRL;
	if (oldstat) {
		*oldstat = stat;
	}

	if (stat) {
		return 0;
	}

	return KE_CPUDI;
}

int
CpuResumeIntr(int oldstat)
{
	*I_CTRL = oldstat;
	return 0;
}

int
CpuDisableIntr()
{
	int stat;
	int ret;

	stat = func_000003DC();

	ret = KE_OK;
	if (!stat) {
		ret = KE_CPUDI;
	}

	return ret;
}

int CpuEnableIntr() {
	func_00000418();
	func_000003F0();

	return 0;
}

int
func_000003DC()
{
	return *I_CTRL;
}

void
func_000003F0()
{
	*I_CTRL = 1;
}

// not sure about this, i'd have thought it'd be in its own .s file
// but then wouldn't it be 16byte aligned
//
// This would not be safe on modern gcc (not that i'm convinced it is on 2.8 either)
int
func_00000408()
{
	asm("li $2, 4 \n"
		"syscall  \n");
}

int
func_00000418()
{
	asm("li $2, 8 \n"
		"syscall  \n");
}

int
func_00000428()
{
	asm("li $2, 0x10 \n"
		"syscall  \n");
}

int
func_00000438()
{
	asm("li $2, 0x14 \n"
		"syscall  \n");
}

int
func_00000448()
{
	asm("li $2, 0xc \n"
		"syscall  \n");
}

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", EnableIntr);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", DisableIntr);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_000007B4);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000854);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_000008FC);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_0000090C);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_0000095C);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", dma_irq);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000D20);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000D30);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000D48);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000D58);

INCLUDE_RODATA("asm/intrmani/nonmatchings/intrman", D_00001570);

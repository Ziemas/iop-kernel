#include "intrman.h"

#include "common.h"
#include "excepman.h"
#include "intrman_internal.h"
#include "loadcore.h"
#include "sys/types.h"

#define I_CFG ((volatile u_int *)0xBF801450)
#define I_STAT ((volatile u_int *)0xBF801070)
#define I_MASK ((volatile u_int *)0xBF801074)
#define I_CTRL ((volatile u_int *)0xBF801078)
#define D_DICR ((volatile u_int *)0xBF8010F4)
#define D_DICR2 ((volatile u_int *)0xBF801574)

struct intrman {
	int unk0;
	int unk4;
	int unk8;
	int unkc;
	char intr_stack[0x800];
};

struct intrman im;

void exc_irq();
void exc_irq_fb();
void exc_syscall();
int dma_irq();

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

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", RegisterIntrHandler);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", ReleaseIntrHandler);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000348);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000374);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", CpuDisableIntr);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", CpuEnableIntr);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_000003DC);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_000003F0);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000408);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000418);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000428);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000438);

INCLUDE_ASM("asm/intrmani/nonmatchings/intrman", func_00000448);

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

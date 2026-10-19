#include "loadcore.h"

#include "common.h"
#include "kerror.h"
#include "sys/types.h"

// TODO, loadcore_main uses the string
// ModuleInfo Module = { "Module_Manager", 0x206 };
// char idstring[16] = "PsIIsysmem3000";

struct loadcore {
	libhead *lib;
	int unk4;
	int unk8;
	int unkC;
	int unk10;
	int unk14;
	int unk18;
	int unk1C;
	int unk20;
};

int bootmodes[17];

struct loadcore lc;

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", loadcore_main);

void
RegisterBootMode(u_int *b)
{
	int *modes = *(int **)0x3f4; // mode insert ptr
	int count = (b[0] >> 24) + 1;
	int i;

	if (modes + 1 * count <= &bootmodes[16]) {
		for (i = 0; i < count; i++) {
			*modes++ = *b++;
		}

		*modes = 0;
		*((int **)0x3f4) = modes;
	}
}

int *
QueryBootMode(int mode)
{
	int *mp = *(u_int **)0x3f0; // mode start ptr

	while (*mp) {
		if (mode == ((u_char *)mp)[2]) {
			return mp;
		}

		mp += ((u_char *)mp)[3] + 1;
	}

	return 0;
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_0000073C);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000007E0);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_0000085C);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000008C8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", RegisterLibraryEntries);

int
RegisterNonAutoLinkEntries(libhead *lib)
{
	if (!lib || (u_int)lib->next != 0x41c00000) {
		return KE_ILLEGAL_LIBRARY;
	}

	lib->flags |= 1;
	lib->next = lc.lib;
	lc.lib = lib;
	FlushIcache();

	return KE_OK;
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", ReleaseLibraryEntries);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", QueryLibraryEntryTable);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00000CAC);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00000D68);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00000EB4);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00000EC4);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00000ED8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", SetRebootTimeLibraryHandlingMode);

// TODO could be either libcaller or libhead
int
func_00000FB0(libhead *lib1, libhead *lib2)
{
	return ((u_int *)lib1->name)[0] == ((u_int *)lib2->name)[0] &&
	  (((u_int *)lib1->name)[1] == ((u_int *)lib2->name)[1]);
}

// TODO could be either libcaller or libhead
int
func_00000FE0(libhead *lib1, libhead *lib2)
{
	return (lib1->version >> 8) - (lib2->version >> 8);
}

// TODO could be either libcaller or libhead
int
func_00000FF8(libhead *lib1, libhead *lib2)
{
	return (lib1->version & 0xff) - (lib2->version & 0xff);
}

// TODO could be either libcaller or libhead
int
func_00001008(libhead *lib1, libhead *lib2)
{
	return ((u_int *)lib2->name)[0] == ((u_int *)lib1->name)[0] &&
	  (((u_int *)lib2->name)[1] == ((u_int *)lib1->name)[1]);
}

// TODO could be either libcaller or libhead
int
func_00001038(libhead *lib1, libhead *lib2)
{
	return (lib1->version >> 8) - (lib2->version >> 8);
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001050);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000010A0);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001138);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000011D8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000012A0);

void
func_0000130C(void *start, void *end, u_int val)
{
	u_int *p = start;
	u_int *pe = end;

	do {
		*p++ = val;
	} while (p < pe);
}

// strlen
int
func_00001328(const char *s)
{
	int len = 0;

	if (!s) {
		return 0;
	}
	while (*s++) {
		len++;
	}

	return len;
}

// memmove
void *
func_00001354(void *dst, void *src, int len)
{
	char *pd, *ps;
	int i;

	pd = dst;
	ps = src;

	if (!dst) {
		return dst;
	}

	if (dst >= src) {
		for (i = len - 1; i >= 0; i--) {
			pd[i] = ps[i];
		}

	} else {
		while (len > 0) {
			*pd++ = *ps++;
			len--;
		}
	}

	return dst;
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000013C8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001410);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001458);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000014A0);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_0000169C);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001734);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000017C4);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001844);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000018A8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001A04);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001B98);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001BD0);

INCLUDE_RODATA("asm/loadcore/nonmatchings/loadcore", D_00001F20);

#include "loadcore.h"

#include "common.h"
#include "kerror.h"
#include "loadcore_internal.h"
#include "sys/types.h"
#include "sysmem.h"

ModuleInfo Module = { "Module_Manager", 0x206 };
char idstring[16] = "PsIIloadcore3000";

// probably suspesnd/resume intr
struct unk_funcs {
	int (*unk0)(int *);
	int (*unk4)(int);

	// TODO figure out arg on unk8 when the source for this
	// struct is decompiled
	int (*unk8)();
};

struct reboot_notify {
	// lowest two bits store priority
	u_int hnd;
	u_int gp;
};

struct iop_mod {
	struct iop_mod *next;
	ModuleInfo *modinfo;
	short version;
	int id;
	u_int entry;
	u_int gp;
	u_int text_addr;
	u_int text_size;
	u_int data_size;
	u_int bss_size;
};

struct loadcore {
	libhead *lib;
	char *unk4;
	int unk8;
	int unkC;
	struct iop_mod *modlist;
	int mod_count;
	int next_id;

	// insert ptr for reboot notifiers
	struct reboot_notify *rbn_ptr;
	struct unk_funcs *unk20;
};

int bootmodes[17];

struct loadcore lc;

extern libhead loadcore_entry;

// LinkLibraryEntries
int func_00000CAC(void *start, u_int size);
int func_00000D68(void *start, u_int size);
int func_00001008(libhead *lib1, LibInfo *lib2);
int func_00001038(libhead *lib1, LibInfo *info);
int func_000010A0(libcaller *client);
int func_00001138(libcaller *client);
void func_0000130C(void *start, void *end, u_int val);

// interrupt things?  would makes ens given the args
int func_000013C8(int *unk);
int func_00001410(int unk);

extern void ftext;
extern void etext;

struct boot_arg {
	u_int memsize;
	u_int bootmode;
	char *command;
	void *start;
	void *rp_buf;
	u_int rp_size;
	int mod_count;
	void **mod_data;
};

#if 1
INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", loadcore_main);
#else
void
loadcore_main(u_int *arg)
{
	int bm[2];
	int i;
	u_int unk[8];
	void *unkb;

	// unk = __builtin_alloca(8 * 4);

	unk[0] = arg[0];
	unk[1] = arg[1];
	unk[3] = arg[3];
	unk[4] = arg[4];
	unk[5] = arg[5];
	unk[6] = arg[6];
	unk[7] = arg[7];
	unk[8] = arg[8];

	lc.lib = (void *)unk[3];
	*(u_int **)0x3f0 = bootmodes;
	*(u_int *)0x3f4 = 0x3f4;
	lc.unk20 = 0;
	lc.unk4 = (void *)unk[3];
	lc.unk14 = 2;
	lc.unk8 = 0;
	lc.unkC = 0;
	lc.unk18 = 3;

	for (i = 0; i < 17; i++) {
		bootmodes[i] = 0;
	}

	// bm[0] = BOOTMODE(4, arg->bootmode, 0);
	RegisterBootMode(bm);

	lc.unk10 = (u_int)lc.unk4 - 48;

	func_00000CAC(&ftext, &etext);
	RegisterLibraryEntries(&loadcore_entry);
	AllocSysMemory(2, 0x20c4, (void *)(0x1FFFFFFF << 2));
	if (unk[4]) {
		AllocSysMemory(2, unk[5], (void *)unk[4]);
	}
	unkb = __builtin_alloca(0xd00);
	func_0000130C(unkb, unkb + 0xd00, 0x11111111);

	if (QueryMemSize() >= unkb) {
		AllocSysMemory(2, QueryMemSize() - ((u_int)unkb & (0x1FFFFFFF << 8)), unkb);
	}
}

#endif

void
RegisterBootMode(int *param)
{
	BootParam *p = (BootParam *)param;
	int count = p->extwords + 1;
	int *dst = *(int **)0x3f4; // mode insert ptr
	int *src = (int *)param;
	int i;

	if (dst + 1 * count <= &bootmodes[16]) {
		for (i = 0; i < count; i++) {
			*dst++ = *src++;
		}

		*dst = 0;
		*((int **)0x3f4) = dst;
	}
}

BootParam *
QueryBootMode(int key)
{
	int *mp = *(u_int **)0x3f0; // mode start ptr
	BootParam *param;

	while (*mp) {
		param = (BootParam *)mp;

		if (key == param->key) {
			return param;
		}

		mp += param->extwords + 1;
	}

	return 0;
}

// AddRebootNotifyHandler
int
func_0000073C(int (*hnd)(int *, int), char prio, int *stat)
{
	int s, ret;
	u_int gp;

	s = 1;

	if (!lc.rbn_ptr) {
		ret = hnd(&s, 0);

		if (stat) {
			*stat = ret;
		}

		return 0;
	}

	asm volatile("move %0, $gp \n" : "=r"(gp));

	lc.rbn_ptr->hnd = (u_int)hnd + (prio & 3);
	lc.rbn_ptr->gp = gp;

	lc.rbn_ptr[1].hnd = 0;
	lc.rbn_ptr++;

	return 1;
}

// RegisterModule
void
func_000007E0(struct iop_mod *mod)
{
	struct iop_mod **pp;

	for (pp = &lc.modlist; *pp; pp = &(*pp)->next) {
		if (*pp >= mod) {
			break;
		}
	}

	mod->next = *pp;
	*pp = mod;
	mod->id = lc.next_id++;
	lc.mod_count++;
}

// ReleaseModule
void
func_0000085C(struct iop_mod *mod)
{
	struct iop_mod **pp;

	if (!mod) {
		return;
	}

	for (pp = &lc.modlist; *pp; pp = &(*pp)->next) {
		if (*pp == mod) {
			*pp = (*pp)->next;
			lc.mod_count--;
			return;
		}
	}
}

struct iop_mod *
func_000008C8(void *ptr)
{
	struct iop_mod *mod;

	for (mod = lc.modlist; mod; mod = mod->next) {
		if (ptr >= mod->text_addr &&
		  (ptr < mod->text_addr + mod->text_size + mod->data_size + mod->bss_size)) {
			return mod;
		}
	}

	return NULL;
}

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

int
ReleaseLibraryEntries(libhead *lib)
{
	libhead **pp;
	int unk;

	func_000013C8(&unk);

	for (pp = &lc.lib; *pp; pp = &(*pp)->next) {
		if (*pp == lib) {
			break;
		}
	}

	if (*pp != lib) {
		func_00001410(unk);
		return KE_LIBRARY_NOTFOUND;
	}

	if (!lib->client) {
		lib->client = 0;
		*pp = lib->next;
		*(u_int *)&lib->next = 0x41c00000;

		func_00001410(unk);
		return KE_OK;
	}

	func_00001410(unk);
	return KE_LIBRARY_INUSE;
}

void *
QueryLibraryEntryTable(LibInfo *libinfo)
{
	libhead *p;

	p = lc.lib;

	while (p) {
		if (func_00001008(p, libinfo)) {
			if (!func_00001038(p, libinfo)) {
				return &p[1];
			}
		}

		p = p->next;
	}

	return 0;
}

int
func_00000CAC(void *start, u_int size)
{
	libcaller *client;
	int count;
	u_int *p;
	int i = size / 4;

	// ??
	count = i;
	p = start;

	for (i = count; i > 0; i--, p++) {
		if (*p == LIBMAGIC) {
			client = (libcaller *)p;

			if (func_000010A0(client) && !(client->flags & 7)) {
				if (func_00001138(client)) {
					func_00000D68(start, size);
					return -1;
				}
			}
		}
	}
	return 0;
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00000D68);

struct loadcore *
func_00000EB4()
{
	return &lc;
}

void
func_00000EC4(libcaller *client)
{
	client->flags |= 1;
}

void
func_00000ED8(libcaller *client)
{
	client->flags &= ~1;
}

int
SetRebootTimeLibraryHandlingMode(libhead *lib, int mode)
{
	libhead **pp;
	int unk;

	if (!lib) {
		return KE_ILLEGAL_LIBRARY;
	}

	func_000013C8(&unk);

	for (pp = &lc.lib; *pp; pp = &(*pp)->next) {
		if (*pp == lib) {
			break;
		}
	}

	if (*pp != lib && lib->next != 0x41c00000) {
		func_00001410(unk);
		return KE_LIBRARY_NOTFOUND;
	}

	lib->flags = (lib->flags & ~6) | (mode & 6);
	func_00001410(unk);
	return 0;
}

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

int
func_00001008(libhead *lib1, LibInfo *info)
{
	return ((u_int *)info->name)[0] == ((u_int *)lib1->name)[0] &&
	  (((u_int *)info->name)[1] == ((u_int *)lib1->name)[1]);
}

int
func_00001038(libhead *lib1, LibInfo *info)
{
	return (lib1->version >> 8) - (info->version >> 8);
}

void
func_00001050(libcaller *client)
{
	u_int *instr = (u_int *)&client[1];

	while (*instr) {
		if (instr[1] >> 26 != 9) {
			break;
		}

		// TODO what instruction is this?
		instr[0] = 0x3E00008;
		instr += 2;
	}
}

int
func_000010A0(libcaller *client)
{
	u_int *instr;

	if (client->magic != 0x41e00000) {
		return 0;
	}

	instr = (u_int *)&client[1];

	while (*instr) {
		if (instr[1] >> 26 != 9 || (instr[0] != 0x3E00008 && instr[0] >> 26 != 2)) {
			return 0;
		}

		instr += 2;
	}

	return !instr[1] && &client[1] < instr;
}

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

int
func_000013C8(int *unk)
{
	if (lc.unk20 && lc.unk20->unk0) {
		return lc.unk20->unk0(unk);
	}

	return 0;
}

int
func_00001410(int unk)
{
	if (lc.unk20 && lc.unk20->unk4) {
		return lc.unk20->unk4(unk);
	}

	return 0;
}

int
func_00001458()
{
	if (lc.unk20 && lc.unk20->unk8) {
		return lc.unk20->unk8();
	}

	return 0;
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000014A0);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_0000169C);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001734);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000017C4);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001844);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000018A8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001A04);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001B98);

//  memset32
void
func_00001BD0(void *p, int len)
{
	u_int *ptr = p;

	while (len > 0) {
		*ptr++ = 0;
		len--;
	}
}

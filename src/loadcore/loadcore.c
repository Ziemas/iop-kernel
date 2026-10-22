#include "loadcore.h"

#include "coff.h"
#include "common.h"
#include "elf32.h"
#include "kerror.h"
#include "loadcore_internal.h"
#include "sys/types.h"
#include "sysmem.h"

ModuleInfo Module = { "Module_Manager", 0x206 };
char idstring[16] = "PsIIloadcore3000";

struct intr_funcs {
	int (*suspend_intr)(int *);
	int (*resume_intr)(int);
	int (*query_intr_ctx)();
};

struct reboot_notify {
	// lowest two bits store priority
	u_int hnd;
	u_int gp;
};

struct unk_thing {
	int unk0;
	struct unk_thing *unk4;
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

	// What is this thing? I can't find a single thing that writes to it
	struct unk_thing unk8;

	struct iop_mod *modlist;
	int mod_count;
	int next_id;

	// insert ptr for reboot notifiers
	struct reboot_notify *rbn_ptr;
	struct intr_funcs *intr_func;
};

int bootmodes[17];

struct loadcore lc;

extern libhead loadcore_entry;

// LinkLibraryEntries
int func_00000CAC(void *start, u_int size);
int func_00000D68(void *start, u_int size);
int func_00000FB0(libhead *lib1, libhead *lib2);
int func_00000FE0(libhead *lib1, libhead *lib2);
int func_00000FF8(libhead *lib1, libhead *lib2);
int func_00001008(libhead *lib, LibInfo *info);
int func_00001038(libhead *lib, LibInfo *info);
void func_00001050(libcaller *client);
int func_000010A0(libcaller *client);
int func_00001138(libcaller *client);
int func_000011D8(libcaller *client, libhead *lib);
int func_000012A0(libhead *lib, libcaller *client);

void lc_fill32(void *start, void *end, u_int val);

// interrupt things?  would makes ens given the args
int suspend_intr(int *unk);
int resume_intr(int unk);

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
		if ((u_int)ptr >= mod->text_addr &&
		  ((u_int)ptr < mod->text_addr + mod->text_size + mod->data_size + mod->bss_size)) {
			return mod;
		}
	}

	return NULL;
}

int
RegisterLibraryEntries(libhead *lib)
{
	libhead *it;
	libcaller *client, *migrate_list, **c_pp, *cnext, *v0;
	int unk;

	if (!lib || (u_int)lib->next != 0x41c00000) {
		return KE_ILLEGAL_LIBRARY;
	}

	suspend_intr(&unk);

	migrate_list = NULL;

	for (it = lc.lib; it; it = it->next) {
		if (func_00000FB0(lib, it) && !func_00000FE0(lib, it)) {
			if (func_00000FF8(lib, it) <= 0) {
				resume_intr(unk);
				return KE_LIBRARY_FOUND;
			}

			// We've registered a newer version of a library we already had
			// figure out which clients of the existing library we can move

			c_pp = &it->client;
			client = it->client;
			it->client = 0;

			for (; client; client = cnext) {
				cnext = client->client;

				// check if the client is locked
				if (!(client->flags & 1)) {
					// else we move them into the migration list
					client->client = migrate_list;
					migrate_list = client;
				} else {
					// use c_pp to insert the locked clients into the previous library list
					*c_pp = client;
					c_pp = &client->client;
					*c_pp = 0;
				}
			}
		}
	}

	// idk what the hell this is
	client = (libcaller *)&lc.unk8;
	while (client->client) {
		if (func_00001008(lib, (LibInfo *)client->client) &&
		  !func_00001038(lib, (LibInfo *)client->client)) {

			v0 = client->client;
			cnext = v0->client;

			v0->client = migrate_list;
			migrate_list = client->client;

			client->client = cnext;
		} else {
			client = client->client;
		}
	}

	// Move clients in migration list to new library
	lib->client = 0;
	while (migrate_list) {
		client = migrate_list;
		cnext = migrate_list->client;

		func_000011D8(client, lib);

		client->client = lib->client;
		lib->client = client;
		migrate_list = cnext;
	}

	lib->flags &= ~1;
	lib->next = lc.lib;
	lc.lib = lib;
	resume_intr(unk);
	FlushIcache();
	return 0;
}

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

	suspend_intr(&unk);

	for (pp = &lc.lib; *pp; pp = &(*pp)->next) {
		if (*pp == lib) {
			break;
		}
	}

	if (*pp != lib) {
		resume_intr(unk);
		return KE_LIBRARY_NOTFOUND;
	}

	if (!lib->client) {
		lib->client = 0;
		*pp = lib->next;
		*(u_int *)&lib->next = 0x41c00000;

		resume_intr(unk);
		return KE_OK;
	}

	resume_intr(unk);
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

int
func_00000D68(void *start, u_int size)
{
	libcaller *client;
	libhead *lib, *lnext;
	void *end;

	end = start + 4 * (size >> 2);

	for (lib = lc.lib; lib; lib = lnext) {
		lnext = lib->next;

		for (client = lib->client; client; client = client->client) {
			if (client >= start && client < end) {
				if (func_000012A0(lib, client)) {
					return -1;
				}

				client->flags &= ~7;
				func_00001050(client);
			}
		}

		if (lib >= start && lib < end) {
			ReleaseLibraryEntries(lib);
		}
	}

	// as usual, don't know whats actually up with this thing
	client = (libcaller *)&lc.unk8;
	while (client->client) {
		libcaller *next, *n, *nn;

		next = client->client;
		if (next >= start && next < end) {
			next->flags &= ~7;
			func_00001050(client->client);
			n = client->client;
			nn = n->client;
			n->client = 0;
			client->client = nn;
		} else {
			client = client->client;
		}
	}

	return 0;
}

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

	suspend_intr(&unk);

	for (pp = &lc.lib; *pp; pp = &(*pp)->next) {
		if (*pp == lib) {
			break;
		}
	}

	if (*pp != lib && (u_int)lib->next != 0x41c00000) {
		resume_intr(unk);
		return KE_LIBRARY_NOTFOUND;
	}

	lib->flags = (lib->flags & ~6) | (mode & 6);
	resume_intr(unk);
	return 0;
}

// compare name
int
func_00000FB0(libhead *lib1, libhead *lib2)
{
	return ((u_int *)lib1->name)[0] == ((u_int *)lib2->name)[0] &&
	  (((u_int *)lib1->name)[1] == ((u_int *)lib2->name)[1]);
}

// compare major versoin
int
func_00000FE0(libhead *lib1, libhead *lib2)
{
	return (lib1->version >> 8) - (lib2->version >> 8);
}

// compare minor version
int
func_00000FF8(libhead *lib1, libhead *lib2)
{
	return (lib1->version & 0xff) - (lib2->version & 0xff);
}

int
func_00001008(libhead *lib, LibInfo *info)
{
	return ((u_int *)info->name)[0] == ((u_int *)lib->name)[0] &&
	  (((u_int *)info->name)[1] == ((u_int *)lib->name)[1]);
}

int
func_00001038(libhead *lib, LibInfo *info)
{
	return (lib->version >> 8) - (info->version >> 8);
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

	return !instr[1] && (u_int *)&client[1] < instr;
}

int
func_00001138(libcaller *client)
{
	libhead *lib;

	for (lib = lc.lib; lib; lib = lib->next) {
		if (!(lib->flags & 1) && func_00001008(lib, (LibInfo *)client) &&
		  !func_00001038(lib, (LibInfo *)client)) {

			func_000011D8(client, lib);
			client->client = lib->client;
			lib->client = client;
			FlushIcache();
			return 0;
		}
	}

	return KE_ERROR;
}

int
func_000011D8(libcaller *client, libhead *lib)
{
	u_int *exports, *insn;
	u_int ordinal;
	int export_count, i;

	exports = (u_int *)&lib[1];
	insn = (u_int *)&client[1];

	export_count = 0;
	while (exports[export_count]) {
		export_count++;
	}

	while (*insn) {
		if (insn[1] >> 26 != 9) {
			break;
		}

		ordinal = insn[1] & 0xffff;
		if (ordinal < export_count) {
			insn[0] = (exports[ordinal] >> 2) & 0x3FFFFFF | 0x8000000;
		} else {
			insn[0] = 0x03E00008;
		}

		insn += 2;
	}

	client->flags = (client->flags & ~4) | 2;
	return 0;
}

int
func_000012A0(libhead *lib, libcaller *client)
{
	libcaller *it, *next;

	if (lib->client == client) {
		lib->client = client->client;
		return 0;
	}

	for (it = lib->client; it->client; it = next) {
		next = it->client;

		if (next == client) {
			it->client = next->client;
			next->client = 0;
			return 0;
		}
	}

	return -1;
}

void
lc_fill32(void *start, void *end, u_int val)
{
	u_int *p = start;
	u_int *pe = end;

	do {
		*p++ = val;
	} while (p < pe);
}

int
lc_strlen(const char *s)
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

void *
lc_memmove(void *dst, void *src, int len)
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
suspend_intr(int *unk)
{
	if (lc.intr_func && lc.intr_func->suspend_intr) {
		return lc.intr_func->suspend_intr(unk);
	}

	return 0;
}

int
resume_intr(int unk)
{
	if (lc.intr_func && lc.intr_func->resume_intr) {
		return lc.intr_func->resume_intr(unk);
	}

	return 0;
}

int
query_intr_context()
{
	if (lc.intr_func && lc.intr_func->query_intr_ctx) {
		return lc.intr_func->query_intr_ctx();
	}

	return 0;
}

int
func_000014A0(void *buf, struct unk_result *a2)
{
	struct coff_fhdr *coff_hdr;
	struct coff_opthdr *coff_opt;
	struct coff_shdr *coff_shdr;
	Elf32_Ehdr *elf_hdr;
	Elf32_Phdr *elf_phdr;
	struct IopMod *mod;

	coff_hdr = buf;
	coff_opt = (struct coff_opthdr *)&coff_hdr[1];
	coff_shdr = (struct coff_shdr *)&coff_opt[1];
	if (coff_hdr->f_magic == 0x162 && coff_opt->magic == 0x107 && coff_hdr->f_nscns < 0x20 &&
	  (coff_hdr->f_flags & 0x2) == 0x2 && coff_hdr->f_opthdr == 0x38 &&
	  coff_shdr->s_paddr == coff_opt->text_start) {

		if (coff_opt->vstamp == 0x7001) {
			return -1;
		}

		a2->type = 1;
		a2->entry = coff_opt->entry;
		a2->gp = coff_opt->gp_value;
		a2->vaddr = coff_opt->text_start;
		a2->text_size = coff_opt->tsize;
		a2->data_size = coff_opt->dsize;
		a2->bss_size = coff_opt->bsize;
		a2->memsz = coff_opt->bss_start + coff_opt->bsize - coff_opt->text_start;
		a2->modinfo = coff_opt->cprmask[3];

		return a2->type;
	}

	elf_hdr = buf;
	elf_phdr = buf + elf_hdr->e_phoff;
	if (elf_hdr->e_ident[4] == 1 && elf_hdr->e_ident[5] == 1 && elf_hdr->e_machine == EM_MIPS &&
	  elf_hdr->e_phentsize == 32 && elf_hdr->e_phnum == 2 && elf_phdr->p_type == 0x70000080 &&
	  (elf_hdr->e_type == 0xff80 || elf_hdr->e_type == 0xff81 || elf_hdr->e_type == 2)) {

		if (elf_hdr->e_type == 0xff80 || elf_hdr->e_type == 0xff81) {
			a2->type = 4;
		} else if (elf_hdr->e_type == 2) {
			a2->type = 3;
		}

		mod = buf + elf_phdr->p_offset;

		a2->entry = mod->entry;
		a2->gp = mod->gp_value;
		a2->vaddr = elf_phdr[1].p_vaddr;
		a2->text_size = mod->text_size;
		a2->data_size = mod->data_size;
		a2->bss_size = mod->bss_size;
		a2->memsz = elf_phdr[1].p_memsz;
		a2->modinfo = mod->moduleinfo;

		return a2->type;
	}

	a2->type = -1;
	return -1;
}

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_0000169C);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001734);

// load coff
INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000017C4);

// load static elf
INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001844);

// load relocatable elf
INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000018A8);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001A04);

INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001B98);

void
lc_memzero32(void *p, int len)
{
	u_int *ptr = p;

	while (len > 0) {
		*ptr++ = 0;
		len--;
	}
}

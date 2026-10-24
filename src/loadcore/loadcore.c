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
	short unkA;
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

int func_0000073C(int (*hnd)(int *, int), char prio, int *stat);
void func_000007E0(struct iop_mod *mod);
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
int func_000014A0(void *buf, struct exe_info *info);
int func_0000169C(void *buf, struct exe_info *info);
void func_00001734(struct exe_info *info, struct iop_mod *mod);
void func_000017C4(void *buf);
void func_00001844(void *buf);
void func_000018A8(void *buf, struct exe_info *info);
void func_00001A04();
void lc_fill32(void *start, void *end, u_int val);

void *lc_memmove(void *dst, void *src, int len);
void lc_memcpy32(void *src, void *dst, u_int size);
void lc_memzero32(void *p, int len);

int lc_strlen(const char *s);

// interrupt things?  would makes ens given the args
int suspend_intr(int *unk);
int resume_intr(int unk);

extern void _ftext;
extern void _etext;
extern void _end;

struct boot_arg {
	u_int memsize;
	u_int mode;
	void *arg;
	void *load_base;
	void *rp_buf;
	u_int rp_size;
	int mod_count;
	void **mod_data;
};

#if 1
INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", loadcore_main);
#else
void
loadcore_main(u_int *in)
{
	struct boot_arg arg_st;
	struct loadcore *core;
	struct iop_mod *sysmem_mod;
	struct iop_mod *lc_mod;
	struct iop_mod *mod;
	struct boot_arg *arg;
	int bm[2];
	int i;
	u_int unk[8];
	void *unkb;
	u_int unkarr[0xd0];
	char arg_copy[8];
	void *mptr;
	int v8;
	int v10;
	void *mp;
	int type;
	struct exe_info info;
	void *modalloc;
	int ret;
	void *aligned_start;
	void *tmp_alloc;
	void *sp1, *sp2;
	char *argstr;
	void *addr;

	arg = &arg_st;

	arg->memsize = in[0];
	arg->mode = in[1];
	arg->arg = in[2];
	arg->load_base = in[3];
	arg->rp_buf = in[4];
	arg->rp_size = in[5];
	arg->mod_count = in[6];
	arg->mod_data = in[7];

	// arg = *_arg;

	lc.lib = arg->arg;

	*(u_int **)0x3f0 = bootmodes;
	*(u_int **)0x3f4 = bootmodes;

	lc.intr_func = NULL;
	lc.unk4 = arg->arg;
	lc.mod_count = 2;
	lc.unk8.unk0 = 0;
	lc.unk8.unk4 = 0;
	lc.next_id = 3;

	for (i = 0; i < 17; i++) {
		bootmodes[i] = 0;
	}

	bm[0] = BOOTMODE(4, arg->mode, 0);
	RegisterBootMode(bm);

	// the iop_mod struct precedes the module .text
	lc_mod = &_ftext - 0x30;
	sysmem_mod = (struct iop_mod *)lc.unk4 - 0x30;
	lc.modlist = lc_mod;

	sysmem_mod->id = 1;
	sysmem_mod->unkA = 3;
	sysmem_mod->next = lc_mod;
	lc_mod->id = 2;
	lc_mod->unkA = 3;

	// scan our text section for imports to resolve
	func_00000CAC(&_ftext, &_etext - &_ftext);
	RegisterLibraryEntries(&loadcore_entry);

	// Register the memory we occupy with sysmem
	aligned_start = (void *)(((int)&_ftext >> 8) << 8);
	AllocSysMemory(SMEM_Addr, &_end - aligned_start, (void *)((u_int)aligned_start & 0x1fffffff));
	if (arg->load_base) {
		// temporarily allocate the ioprp buffer
		tmp_alloc = AllocSysMemory(SMEM_Addr, (u_int)arg->rp_size, arg->load_base);
	}

	// ???
	sp1 = __builtin_alloca(0x10);
	// sp2 = __builtin_alloca(0xdf0+010);
	sp2 = sp1 - 0xe00;
	lc_fill32((int)sp2 & 0x1fffff00, sp1 - 0xf0, 0x11111111);

	// At this point our stack grows down from end of RAM, so use we can figure out
	// the size to allocate the stack space by subtracting from QueryMemSize
	if (QueryMemSize() < unkarr) {
		AllocSysMemory(SMEM_Addr, QueryMemSize() - (u_int)sp2, sp2);
	}

	if (arg->arg) {
		int len = lc_strlen(arg->arg) + 1;
		argstr = __builtin_alloca(len);

		lc_memmove(argstr, arg->arg, len);
		bm[0] = BOOTMODE(5, 0, 1);
		bm[1] = (u_int)argstr;
		RegisterBootMode(bm);
	}

	mptr = __builtin_alloca((arg->mod_count + 1) * 4);
	lc_memmove(mptr, arg->mod_data, (arg->mod_count + 1) * 4);

	lc.rbn_ptr = __builtin_alloca(arg->mod_count * sizeof(*lc.rbn_ptr));

	addr = QueryBlockTopAddress(0);
	while (addr != -1) {
		int BlockSize = QueryBlockSize(addr);

		// shift + and, but how?
		if (BlockSize < 0) {
			lc_fill32(addr, addr + (BlockSize & 0x7FFFFFFF), 0x400D);
		}

		addr = QueryBlockTopAddress(addr + (BlockSize & 0x7fffffff));
	}

	mp = mptr;
	while (mp) {
		type = func_000014A0(mp, &info);
		switch (type) {
		case 1:
			modalloc = AllocSysMemory(SMEM_Addr, info.memsz + info.vaddr, (void *)info.vaddr);
			break;
		case 3:
			modalloc = AllocSysMemory(SMEM_Addr, info.memsz + info.vaddr, (void *)info.vaddr);
			break;
		case 4:
			modalloc = AllocSysMemory(SMEM_Addr, info.memsz + 48, (void *)info.vaddr);
			break;
		}

		if (modalloc) {
			info.vaddr = (u_int)modalloc + 48;
			func_0000169C((void *)info.vaddr, &info);
			if (func_00000CAC((void *)info.vaddr, info.text_size) == 0) {
				FlushIcache();
				asm volatile("move $gp,%0 \n" : : "r"(info.gp));
				ret = ((int (*)(int, int))info.entry)(0, 0);
				if ((ret & 3) == 1) {
					func_00000D68((void *)info.vaddr, info.text_size);
					FreeSysMemory((void *)(((info.vaddr - 0x30) >> 8) << 8));
					continue;
				}

				mod = info.vaddr - 48;
				func_000007E0(mod);
				mod->unkA = 3;
				if ((ret & 3) == 2) {
					mod->unkA = 19;
				}

				if (ret & ~3) {
					func_0000073C((void *)(ret & ~3), 2, 0);
				}

			} else {
				FreeSysMemory(modalloc);
			}
		}

		mp++;
	}

	if (tmp_alloc) {
		lc_fill32(tmp_alloc, tmp_alloc + arg->rp_size, 0x400D);
		FreeSysMemory(tmp_alloc);
	}

	while (1) {
		*(volatile char *)0x80000000 = 2;
	}
}

#endif

void
func_0000067C()
{
}

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
			if ((void *)client >= start && (void *)client < end) {
				if (func_000012A0(lib, client)) {
					return -1;
				}

				client->flags &= ~7;
				func_00001050(client);
			}
		}

		if ((void *)lib >= start && (void *)lib < end) {
			ReleaseLibraryEntries(lib);
		}
	}

	// as usual, don't know whats actually up with this thing
	client = (libcaller *)&lc.unk8;
	while (client->client) {
		libcaller *next, *n, *nn;

		next = client->client;
		if ((void *)next >= start && (void *)next < end) {
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
func_000014A0(void *buf, struct exe_info *info)
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

		info->type = 1;
		info->entry = coff_opt->entry;
		info->gp = coff_opt->gp_value;
		info->vaddr = coff_opt->text_start;
		info->text_size = coff_opt->tsize;
		info->data_size = coff_opt->dsize;
		info->bss_size = coff_opt->bsize;
		info->memsz = coff_opt->bss_start + coff_opt->bsize - coff_opt->text_start;
		info->modinfo = coff_opt->cprmask[3];

		return info->type;
	}

	elf_hdr = buf;
	elf_phdr = buf + elf_hdr->e_phoff;
	if (elf_hdr->e_ident[4] == 1 && elf_hdr->e_ident[5] == 1 && elf_hdr->e_machine == EM_MIPS &&
	  elf_hdr->e_phentsize == 32 && elf_hdr->e_phnum == 2 && elf_phdr[0].p_type == 0x70000080 &&
	  (elf_hdr->e_type == 0xff80 || elf_hdr->e_type == 0xff81 || elf_hdr->e_type == 2)) {

		if (elf_hdr->e_type == 0xff80 || elf_hdr->e_type == 0xff81) {
			info->type = 4;
		} else if (elf_hdr->e_type == 2) {
			info->type = 3;
		}

		mod = buf + elf_phdr[0].p_offset;

		info->entry = mod->entry;
		info->gp = mod->gp_value;
		info->vaddr = elf_phdr[1].p_vaddr;
		info->text_size = mod->text_size;
		info->data_size = mod->data_size;
		info->bss_size = mod->bss_size;
		info->memsz = elf_phdr[1].p_memsz;
		info->modinfo = mod->moduleinfo;

		return info->type;
	}

	info->type = -1;
	return -1;
}

int
func_0000169C(void *buf, struct exe_info *info)
{
	switch (info->type) {
	case 1:
		func_000017C4(buf);
		break;
	case 3:
		func_00001844(buf);
		break;
	case 4:
		func_000018A8(buf, info);
		break;
	default:
		return -1;
	}

	func_00001734(info, (struct iop_mod *)(info->vaddr - 48));
	return 0;
}

void
func_00001734(struct exe_info *info, struct iop_mod *mod)
{
	ModuleInfo **minfo;

	mod->next = NULL;
	mod->modinfo = NULL;
	mod->version = 0;
	mod->unkA = 0;
	mod->id = 0;

	minfo = (ModuleInfo **)info->modinfo;
	if (info->modinfo + 1 != 0) {
		// TODO need to investigate what this actually points to,
		// this doesn't seem right?

		mod->modinfo = *minfo;
		mod->version = ((short *)info->modinfo)[2];
	}

	mod->entry = info->entry;
	mod->gp = info->gp;
	mod->text_addr = info->vaddr;
	mod->text_size = info->text_size;
	mod->data_size = info->data_size;
	mod->bss_size = info->bss_size;
}

// load ecoff
void
func_000017C4(void *buf)
{
	struct coff_fhdr *hdr;
	struct coff_opthdr *opt;
	struct coff_shdr *shdr;
	void *scn;

	hdr = buf;
	opt = (struct coff_opthdr *)&hdr[1];
	shdr = (struct coff_shdr *)&opt[1];

	scn = buf + shdr[0].s_scnptr;
	lc_memcpy32(scn, (void *)opt->text_start, opt->tsize);
	lc_memcpy32(scn + opt->tsize, (void *)opt->data_start, opt->dsize);
	if (opt->bss_start && opt->bsize) {
		lc_memzero32((void *)opt->bss_start, opt->bsize >> 2);
	}
}

// load static elf
void
func_00001844(void *buf)
{
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;

	ehdr = buf;
	phdr = buf + ehdr->e_phoff;

	lc_memcpy32(buf + phdr[1].p_offset, (void *)phdr[1].p_vaddr, phdr[1].p_filesz);
	if (phdr[1].p_memsz > phdr[1].p_filesz) {
		lc_memzero32((void *)phdr[1].p_vaddr + phdr[1].p_filesz,
		  (phdr[1].p_memsz - phdr[1].p_filesz) >> 2);
	}
}

// load relocatable elf
// INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_000018A8);
void
func_000018A8(void *buf, struct exe_info *info)
{
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	Elf32_Shdr *shdr;
	int vaddr;
	int i;

	vaddr = info->vaddr;

	ehdr = buf;
	phdr = buf + ehdr->e_phoff;
	shdr = buf + ehdr->e_shoff;

	info->entry += vaddr;
	info->gp += vaddr;

	if (info->modinfo + 1 != 0) {
		info->modinfo += (u_int)vaddr;
	}

	lc_memcpy32(buf + phdr[1].p_offset, (void *)info->vaddr, phdr[1].p_filesz);
	if (phdr[1].p_memsz > phdr[1].p_filesz) {
		lc_memzero32((void *)info->vaddr + phdr[1].p_filesz,
		  (phdr[1].p_memsz - phdr[1].p_filesz) >> 2);
	}

	for (i = 1; i < ehdr->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_REL) {
			func_00001A04((void *)vaddr, (void *)shdr[i].sh_offset + (int)buf,
			  shdr[i].sh_size / shdr[i].sh_entsize, shdr[(shdr[i].sh_info)].sh_addr,
			  shdr[(shdr[i].sh_info)].sh_size);
		}
	}
}

#if 1
INCLUDE_ASM("asm/loadcore/nonmatchings/loadcore", func_00001A04);
#else
// apply_elf_rel_section
void
func_00001A04(void *buf, Elf32_Rel *rel, int count, int sh_addr, int sh_size)
{
	u_int base, fixed;
	u_int *roffset;
	int insn;
	int disp;
	int i;

	base = (u_int)buf;

	// A = addend (relocated field holds addend in Rel sections)
	// P = address of the field being relocated

	for (i = 0; i < count; i++, rel++) {
		roffset = buf + rel->r_offset;

		switch (ELF32_R_TYPE(rel->r_info)) {
		case R_MIPS_NONE:
			break;
		case R_MIPS_16:
			// S + sign-extend(A)

			fixed = (((int)*roffset << 16) >> 16) + base;

			*roffset &= 0xffff0000;
			*roffset |= fixed & 0xffff;

			break;
		case R_MIPS_32:
			// S + A
			*roffset += base;
			break;
		case R_MIPS_26:
			// (((A << 2) | (P & 0xf0000000) + S) >> 2

			fixed = (*roffset & 0x03ffffff << 2);
			fixed |= (u_int)roffset & 0xf0000000;
			fixed += base;

			*roffset &= 0xfc000000;
			// bugged attempt to sign extend?
			// these are u_int
			*roffset |= ((fixed << 4) >> 4) >> 2;
			break;
		case R_MIPS_HI16:
			//((AHL + S) – (short)(AHL + S)) >> 16
			insn = *roffset & 0xffff0000;
			disp = (*roffset & 0x0000ffff) << 16;
			*roffset = insn | ((disp + base) >> 16);
			break;
		case R_MIPS_LO16:
			// AHL + S
			insn = *roffset & 0xffff0000;
			disp = (short)*roffset & 0x0000ffff;
			*roffset = insn | (disp + base);
			break;
		case 0xfa: // R_MIPSSCE_MHI16 and R_MIPSSCE_ADDEND
			break;
		default:
			break;
		}
	}
}
#endif

// should maybe call this something else to draw attention to the weird arg order
void
lc_memcpy32(void *src, void *dst, u_int size)
{
	u_int *rp = src;
	u_int *wp = dst;
	u_int *end = &rp[size >> 2];

	while (rp < end) {
		*wp++ = *rp++;
	}
}

void
lc_memzero32(void *p, int len)
{
	u_int *ptr = p;

	while (len > 0) {
		*ptr++ = 0;
		len--;
	}
}

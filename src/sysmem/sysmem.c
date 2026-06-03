#include "common.h"

#include <sysmem.h>

#include <loadcore.h>
#include <sys/types.h>

ModuleInfo Module = { "System_Memory_Manager", 0x203 };

char idstring[16] = "PsIIsysmem  3000";

struct mem_block {
    struct mem_block* next;
    u_int in_use : 1;
    u_int address : 15;
    u_int unk : 1;
    u_int size : 15;
};

struct block_list {
    struct block_list* next;
    struct mem_block blocks[31];
};

struct sysmem_state {
    int mem_size;
    struct block_list* list;
    int (**unk8)();
    int unkC;
    volatile int unk10;
};

struct sysmem_state sysmem;

int sysmem_init();
void* func_0000071C(int type, unsigned long size, void* addr);
void func_00000D30();
int func_00000EE4();
int func_00000F2C();

// 0x10e0 (end of bss) + 0x103?
extern int D_10E0[];

int start(int mem_size)
{
    u_int heap_start, sz;
    int ret;

    if (mem_size > 0x7fff00) {
        mem_size = 0x7fff00;
    }

    heap_start = (((u_int)D_10E0 + 0x103) & ~0xff);
    sz = mem_size & ~0xff;

    sysmem.list = (struct block_list*)heap_start;
    sysmem.mem_size = sz;
    sysmem.unk8 = NULL;
    sysmem.unkC = 0;
    sysmem.unk10 = 0;

    if (heap_start + 0x100 <= sz) {
        heap_start = sysmem_init();
    } else {
        heap_start = 0;
        sysmem.list = 0;
    }

    return heap_start;
}

int sysmem_init()
{
    struct block_list* lst;
    struct mem_block* b;
    void* p;
    int i;

    if (!sysmem.list) {
        return 0;
    }

    lst = sysmem.list;
    lst->next = 0;

    for (i = 0; i < 31; i++) {
        lst->blocks[i].next = &lst->blocks[i + 1];
        lst->blocks[i].address = 0;
        lst->blocks[i].size = 0;
        lst->blocks[i].in_use = 0;
        lst->blocks[i].unk = 0;
    }

    lst->blocks[0].size = sysmem.mem_size / 256;
    lst->blocks[30].next = 0;
    sysmem.unkC = 1;

    p = AllocSysMemory(0, (u_int)sysmem.list, 0);
    // we should have just allocated 0 - sysmem.list
    // so p should be 0
    if (p) {
        sysmem.list = NULL;
        return 0;
    }

    p = AllocSysMemory(0, sizeof(struct block_list), 0);
    if (p != sysmem.list) {
        sysmem.list = NULL;
        return 0;
    }

    lst = p;
    for (b = lst->blocks; b; b = b->next) {
        if (!(b->in_use)) {
            return (b->address) * 256;
        }
    }

    return 0;
}

unsigned long QueryMemSize(void)
{
    if (!sysmem.list) {
        return 0;
    }

    return sysmem.mem_size;
}

unsigned long QueryMaxFreeMemSize(void)
{
    struct mem_block* b;
    unsigned long ret;
    u_int unk;

    ret = 0;
    if (!sysmem.list) {
        return ret;
    }

    func_00000EE4(&unk);

    for (b = sysmem.list->blocks; b; b = b->next) {
        if (!b->in_use && ret < b->size) {
            ret = b->size;
        }
    }

    func_00000F2C(unk);

    return ret * 256;
}

unsigned long QueryTotalFreeMemSize(void)
{
    struct mem_block* b;
    unsigned long ret;
    u_int unk;

    ret = 0;
    if (!sysmem.list) {
        return ret;
    }

    func_00000EE4(&unk);

    for (b = sysmem.list->blocks; b; b = b->next) {
        if (!b->in_use) {
            ret += b->size;
        }
    }

    func_00000F2C(unk);

    return ret * 256;
}

void* AllocSysMemory(int type, unsigned long size, void* addr)
{
    void* ret;
    u_int unk;

    if (sysmem.list && (type == SMEM_Low || type == SMEM_High || type == SMEM_Addr)) {
        func_00000EE4(&unk);

        ret = func_0000071C(type, size, addr);
        func_00000D30();

        func_00000F2C(unk);
        return ret;
    }

    return NULL;
}

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", FreeSysMemory);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", QueryBlockTopAddress);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", QueryBlockSize);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", func_00000580);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", func_0000070C);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", func_0000071C);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", func_00000B74);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", func_00000D30);

INCLUDE_ASM("asm/sysmem/nonmatchings/sysmem", func_00000E80);

int func_00000EE4()
{
    if (sysmem.unk8 && sysmem.unk8[0]) {
        return sysmem.unk8[0]();
    }

    return 0;
}

int func_00000F2C()
{
    if (sysmem.unk8 && sysmem.unk8[1]) {
        return sysmem.unk8[1]();
    }

    return 0;
}

int func_00000F74()
{
    if (sysmem.unk8 && sysmem.unk8[2]) {
        return sysmem.unk8[2]();
    }

    return 0;
}

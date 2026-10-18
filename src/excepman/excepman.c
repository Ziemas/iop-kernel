#include "excepman.h"

#include "common.h"
#include "kerror.h"
#include "loadcore.h"
#include "sys/types.h"
#include "sysmem.h"

ModuleInfo Module = { "Exception_Manager", 0x101 };

extern libhead excepman_entry;

/* Given how weird every single loop over this is I kind of wonder if there
 * even is a struct for this */
struct exc_chain {
	struct exc_chain *next;
	int param;
};

struct excepman {
	int handler_table;
	struct exc_chain *handler[EXCEP_MAX];
	void *default_handler;
	struct exc_chain *chain_pool;
};

struct excepman em;

extern void default_handler;
extern void exception_code;
extern void exception_code_end;

void pool_alloc();
void func_0000038C();
void pool_push(struct exc_chain *h);
struct exc_chain *pool_pop();

int
start()
{
	u_int *dst, *src;
	u_int *end;
	int i;

	for (i = 0; i < EXCEP_MAX; i++) {
		em.handler[i] = 0;
	}

	em.default_handler = 0;
	pool_alloc();
	em.handler_table = 0x440;

	dst = 0;
	src = &exception_code;
	end = &exception_code_end;

	while (src < end) {
		if (*src == 0x8f5a0000 || *src == 0x8f7b0000) {
			*dst++ = *src++ | em.handler_table & 0xffff;
		} else {
			*dst++ = *src++;
		}
	}

	RegisterDefaultExceptionHandler(&default_handler);
	RegisterLibraryEntries(&excepman_entry);
	return 0;
}

int
func_000000E0()
{
	return 0;
}

int
func_000000E8()
{
	func_000000E0();
	return start();
}

int
RegisterExceptionHandler(int excode, void (*handler)())
{
	return RegisterPriorityExceptionHandler(excode, 2, handler);
}

int
RegisterPriorityExceptionHandler(int excode, int priority, void (*handler)())
{
	struct exc_chain *h;
	struct exc_chain *it, *unk;

	h = (struct exc_chain *)handler;

	if (h->next) {
		return KE_EXPHANDLER_USED;
	}

	if (excode >= EXCEP_MAX || excode < 0) {
		return KE_ILLEGAL_EXPCODE;
	}

	priority &= 3;
	unk = pool_pop();
	it = (struct exc_chain *)&em.handler[excode];
	unk->param = ((u_int)handler & ~3) | priority;

	while (it->next) {
		if (((it->next->param) & 3) >= priority) {
			break;
		}

		it = it->next;
	}

	unk->next = it->next;
	it->next = unk;

	func_0000038C();

	return 0;
}

int
RegisterDefaultExceptionHandler(void (*handler)())
{
	void **h = (void **)handler;

	if (h[0]) {
		return KE_EXPHANDLER_USED;
	}

	h[0] = em.default_handler;
	em.default_handler = &h[2];

	func_0000038C();
	return 0;
}

int
ReleaseExceptionHandler(int excode, void (*handler)())
{
	struct exc_chain *it, *it2;

	if (excode >= EXCEP_MAX || excode < 0) {
		return KE_ILLEGAL_EXPCODE;
	}

	it = (struct exc_chain *)&em.handler[excode];
	while (it->next) {
		if (((it->next->param) & ~3) == handler) {
			goto found;
		}

		it = it->next;
	}

	return KE_EXPHANDLER_NOUSE;

found:
	it2 = it->next;
	it->next = it2->next;
	*(u_int *)(it2->param & ~3) = 0;

	pool_push(it2);
	func_0000038C();

	return 0;
}

int
ReleaseDefaultExceptionHandler(void (*handler)())
{
	void **it, **h;

	h = (void **)handler + 2;
	it = &em.default_handler;

	while (*it) {
		if ((*it) == h) {
			goto found;
		}

		it = *it - 8;
	}

	return KE_EXPHANDLER_NOUSE;

found:
	*it = *(h - 2);
	*(h - 2) = 0;

	func_0000038C();

	return 0;
}

struct excepman *
func_0000037C()
{
	return &em;
}

void
func_0000038C()
{
	struct exc_chain *it, *next;
	void **p;
	int i;

	for (i = 0; i < EXCEP_MAX; i++) {
		if (em.handler[i]) {
			it = em.handler[i];
			while (1) {
				if (!it->next) {
					break;
				}

				next = it->next;
				*(u_int *)(it->param & ~3u) = (next->param & ~3) + 8;

				it = it->next;
			}

			*(void **)(it->param & ~3) = em.default_handler;
		}
	}

	p = (void **)em.handler_table;
	for (i = 0; i < EXCEP_MAX; i++) {
		if (!em.handler[i]) {
			p[i] = em.default_handler;
		} else {
			it = em.handler[i];
			p[i] = (void **)(it->param & ~3) + 2;
		}
	}
}

struct exc_chain *
pool_pop()
{
	struct exc_chain *ret;

	if (!em.chain_pool) {
		pool_alloc();
	}

	ret = em.chain_pool;
	if (em.chain_pool) {
		em.chain_pool = ret->next;
	}

	return ret;
}

void
pool_push(struct exc_chain *h)
{
	h->next = em.chain_pool;
	em.chain_pool = h;
}

void
pool_alloc()
{
	struct exc_chain *unk;
	u_int i;

	em.chain_pool = AllocSysMemory(0, sizeof(*unk) * 32, 0);

	unk = em.chain_pool;
	for (i = 0; i < 31; i++) {
		unk[i].next = &unk[i + 1];
	}

	unk[i].next = 0;
}

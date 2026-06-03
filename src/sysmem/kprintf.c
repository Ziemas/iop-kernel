#include <stdarg.h>
#include <sys/types.h>

int (*kprintf_handler)(void* ctx, const char* fmt, va_list va) = NULL;
void* kprintf_context = NULL;

int Kprintf(const char* fmt, ...)
{
    va_list va;
    int ret = 0;

    if (kprintf_handler) {
        va_start(va, fmt);
        ret = kprintf_handler(kprintf_context, fmt, va);
    }

    return ret;
}

void KprintfSet(int (*handler)(void* context, const char* format, va_list ap), void* context)
{
    u_int unk;

    if (kprintf_handler && handler) {
        unk = Kprintf(0);
        ((int (*)(void*, u_int))handler)(context, unk);
    }

    kprintf_context = context;
    kprintf_handler = handler;
}

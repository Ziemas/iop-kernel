#ifndef SYSMEM_INTERNAL_H_
#define SYSMEM_INTERNAL_H_

#include <stdarg.h>

void KprintfSet(int (*)(void *context, const char *format, va_list ap), void *context);

#endif // SYSMEM_INTERNAL_H_

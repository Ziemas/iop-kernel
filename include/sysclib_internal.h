#ifndef SYSCLIB_INTERNAL_H_
#define SYSCLIB_INTERNAL_H_

#include <stdarg.h>

int prnt(void (*)(void *, int), void *context, const char *format, va_list ap);

#endif // SYSCLIB_INTERNAL_H_

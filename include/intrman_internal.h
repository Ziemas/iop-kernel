#ifndef INTRMAN_INTERNAL_H_
#define INTRMAN_INTERNAL_H_

int RegisterIntrHandler(int irq, int mode, int (*handler)(void *), void *arg);
int EnableIntr(int irq);
int CpuInvokeInKmode(void *function, ...);
void DisableDispatchIntr(int irq);

#endif // INTRMAN_INTERNAL_H_

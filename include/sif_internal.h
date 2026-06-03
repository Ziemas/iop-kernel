#ifndef SIF_INTERNAL_H_
#define SIF_INTERNAL_H_

void sceSifDma2Init();
void sceSifDma2Transfer(void *addr, int size, int mode);
void sceSifDma2Sync();
int sceSifDma2Sending();

#endif // SIF_INTERNAL_H_

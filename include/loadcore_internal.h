#ifndef LOADCORE_INTERNAL_H_
#define LOADCORE_INTERNAL_H_

#include "sys/types.h"

int *QueryBootMode(int mode_id);
void RegisterBootMode(int *mode);
int AddRebootNotifyHandler(void* func, int priority, int *stat);

#endif // LOADCORE_INTERNAL_H_

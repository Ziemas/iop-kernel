#ifndef IOPREGS_H_
#define IOPREGS_H_

#include "sys/types.h"

// clang-format off

#define I_STAT     ((volatile u_int   *)(0xBF801070))
#define IRQ_CTRL   ((volatile u_int   *)(0xBF801450))
#define R_BF80101C ((volatile u_int   *)(0xBF80101C))
#define D5_MADR    ((volatile u_int   *)(0xBF8010D0))
#define D5_BCRW    ((volatile u_short *)(0xBF8010D4))
#define D5_BCRN    ((volatile u_short *)(0xBF8010D6))
#define D5_CHCR    ((volatile u_int   *)(0xBF8010D8))
#define D_PCR      ((volatile u_int   *)(0xBF8010F0))
#define D_ICR      ((volatile u_int   *)(0xbf8010f4))
#define D_DMACEN   ((volatile u_int   *)(0xbf801578))
#define R_BF803100 ((volatile u_char  *)(0xBF803100))
#define R_BF803200 ((volatile u_char  *)(0xBF803200))

// clang-format on

#endif // IOPREGS_H_

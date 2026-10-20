#include "cpureg.h"

    .globl start
start:
    mtc0       zero, C0_SR
    lw         v0, 0x0(a0)
    sll        sp, v0, 20
    addu       sp, -0x40
    move       fp, sp
    la         gp, _gp
    j          loadcore_main

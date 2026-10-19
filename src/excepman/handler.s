#include "excepman.h"
#include "cpureg.h"

    .set noat
    .set noreorder

    .text
    .globl exception_code
exception_code:
    // TLB MISS
    /* 0x00 */  nop
    /* 0x04 */  nop
    /* 0x08 */  nop
    /* 0x0C */  nop
    /* 0x10 */  break      1
    /* 0x14 */  break      1
    /* 0x18 */  break      1
    /* 0x1C */  break      1
    /* 0x20 */  break      1
    /* 0x24 */  break      1
    /* 0x28 */  break      1
    /* 0x2C */  break      1
    /* 0x30 */  break      1
    /* 0x34 */  break      1
    /* 0x38 */  break      1
    /* 0x3C */  break      1

    // Hardware Breakpoint
    /* 0x40 */  sw         k0, 0x420(zero)
    /* 0x44 */  mfc0       k1, C0_EPC
    /* 0x48 */  mfc0       k0, C0_CAUSE
    /* 0x4C */  sw         k1, 0x424(zero)
    /* 0x50 */  sw         k0, 0x428(zero)
    /* 0x54 */  mfc0       k1, C0_SR
    /* 0x58 */  mfc0       k0, C0_DCIC
    /* 0x5C */  sw         k1, 0x42C(zero)
    /* 0x60 */  sw         k0, 0x430(zero)
    /* 0x64 */  li         k1, EXCEP_HDB * 4
    /* 0x68 */  lw         k1, 0x0(k1)
    /* 0x6C */  mtc0       zero, C0_DCIC
    /* 0x70 */  jr         k1
    /* 0x74 */   nop
    /* 0x78 */  nop
    /* 0x7C */  nop

    // General exception
    /* 0x80 */  sw         AT, 0x400(zero)
    /* 0x84 */  sw         k0, 0x410(zero)
    /* 0x88 */  mfc0       k0, C0_EPC
    /* 0x8C */  mfc0       AT, C0_SR
    /* 0x90 */  sw         k0, 0x404(zero)
    /* 0x94 */  sw         AT, 0x408(zero)
    /* 0x98 */  mfc0       k0, C0_CAUSE
    /* 0x9C */  nop
    /* 0xA0 */  sw         k0, 0x40C(zero)
    /* 0xA4 */  andi       k0, (EXCEP_MAX - 1) * 4
    /* 0xA8 */  lw         k0, 0x0(k0)
    /* 0xAC */  nop
    /* 0xB0 */  jr         k0
    /* 0xB4 */   nop
    /* 0xB8 */  nop
    /* 0xBC */  nop
    /* 0xC0 */  nop
    /* 0xC4 */  nop
    /* 0xC8 */  nop
    /* 0xCC */  nop

    .globl exception_code_end
exception_code_end:

    .globl default_handler
default_handler:
    .word 0,0
1:
    b   1b
    nop

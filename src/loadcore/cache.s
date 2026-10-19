#include "cpureg.h"

    .set noreorder

    .text
    .globl FlushIcache
FlushIcache:
    // calls _FlushIcache in the unchached segment
    mfc0       t0, C0_SR
    nop
    la         t4, _FlushIcache
    or         t4, K1BASE
    jr         t4
    // missing nop...

    // FlushIcache impl
    .globl _FlushIcache
_FlushIcache:
    mtc0      zero, C0_SR
    nop
    nop
    lw         t6,   D_BF801450
    nop
    li         t7,   ~1
    and        t1,   t6, t7
    sw         t1,   D_BF801450
    lw         zero, D_BF801450
    lw         t7,   D_BF801578
    sw         zero, D_BF801578
    lw         zero, D_BF801578
    lw         t5,   0xFFFE0130
    li         t1,   0xC04 // TAG | IS0 | IS1
    sw         t1,   0xFFFE0130
    li         t4,   SR_ISC
    mtc0       t4,   C0_SR
    nop
    nop
    li         t2, 0x0
    li         t3, 0xF80
1:
    sw         zero, 0x0(t2)
    sw         zero, 0x10(t2)
    sw         zero, 0x20(t2)
    sw         zero, 0x30(t2)
    sw         zero, 0x40(t2)
    sw         zero, 0x50(t2)
    sw         zero, 0x60(t2)
    sw         zero, 0x70(t2)
    bne        t2,   t3, 1b
    add        t2,   0x80

    mtc0       zero, C0_SR
    nop
    sw         t5,   0xFFFE0130
    lw         zero, 0xFFFE0130
    nop
    sw         t7,   D_BF801578
    lw         zero, D_BF801578
    sw         t6,   D_BF801450
    lw         zero, D_BF801450
    mtc0       t0,   C0_SR
    nop
    jr         ra
    nop

    .globl FlushDcache
FlushDcache:
    mfc0       t0, C0_SR
    nop
    la         t4, _FlushDcache
    or         t4, K1BASE
    jr         t4
    // missing nop...

    // FlushDCache impl
    .globl _FlushDcache
_FlushDcache:
    mtc0       zero, C0_SR
    nop
    nop
    lw         t6,   D_BF801450
    nop
    li         t7,   ~1
    and        t1,   t6, t7
    sw         t1,   D_BF801450
    lw         zero, D_BF801450
    lw         t7,   D_BF801578
    sw         zero, D_BF801578
    lw         zero, D_BF801578
    lw         t5,   0xFFFE0130
    li         t1,   0xC4 // TAG | DS | ??? (2 bits like the icache?)
    sw         t1,   0xFFFE0130
    li         t4,   0x10000
    mtc0       t4,   C0_SR
    nop
    nop
    li         t2, 0x0
    li         t3, 0x380
1:
    sw         zero, 0x0(t2)
    sw         zero, 0x10(t2)
    sw         zero, 0x20(t2)
    sw         zero, 0x30(t2)
    sw         zero, 0x40(t2)
    sw         zero, 0x50(t2)
    sw         zero, 0x60(t2)
    sw         zero, 0x70(t2)
    bne        t2, t3, 1b
    add        t2, 0x80

    mtc0       zero, C0_SR
    nop
    sw         t5,   0xFFFE0130
    lw         zero, 0xFFFE0130
    nop
    sw         t7,   D_BF801578
    lw         zero, D_BF801578
    sw         t6,   D_BF801450
    lw         zero, D_BF801450
    mtc0       t0,   C0_SR
    nop
    jr         ra
    nop

    // SetBCC
    .globl func_00001DF8
func_00001DF8:
    mfc0       t0, C0_SR
    nop
    la         t4, func_00001E14
    or         t4, K1BASE
    jr         t4
    // missing nop...

    // SetBCC impl
    .globl func_00001E14
func_00001E14:
     mtc0       zero, C0_SR
     nop
     sw         a0,   0xFFFE0130 // BIU/Cache Configuration
     lw         zero, 0xFFFE0130
     nop
     mtc0       t0,   C0_SR
     nop
     jr         ra
     nop

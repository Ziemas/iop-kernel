#include "cpureg.h"

.set noat
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
    /* 1C0C 00001C0C 00608040 */   mtc0      zero, $12 /* handwritten instruction */
    /* 1C10 00001C10 00000000 */  nop
    /* 1C14 00001C14 00000000 */  nop
    /* 1C18 00001C18 80BF0E3C */  lui        t6, %hi(D_BF801450)
    /* 1C1C 00001C1C 5014CE8D */  lw         t6, %lo(D_BF801450)(t6)
    /* 1C20 00001C20 00000000 */  nop
    /* 1C24 00001C24 FEFF0F24 */  addiu      t7, zero, -0x2
    /* 1C28 00001C28 2448CF01 */  and        t1, t6, t7
    /* 1C2C 00001C2C 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1C30 00001C30 501429AC */  sw         t1, %lo(D_BF801450)(AT)
    /* 1C34 00001C34 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1C38 00001C38 5014208C */  lw         zero, %lo(D_BF801450)(AT)
    /* 1C3C 00001C3C 80BF0F3C */  lui        t7, %hi(D_BF801578)
    /* 1C40 00001C40 7815EF8D */  lw         t7, %lo(D_BF801578)(t7)
    /* 1C44 00001C44 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1C48 00001C48 781520AC */  sw         zero, %lo(D_BF801578)(AT)
    /* 1C4C 00001C4C 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1C50 00001C50 7815208C */  lw         zero, %lo(D_BF801578)(AT)
    /* 1C54 00001C54 FEFF0D3C */  lui        t5, (0xFFFE0130 >> 16)
    /* 1C58 00001C58 3001AD8D */  lw         t5, (0xFFFE0130 & 0xFFFF)(t5)
    /* 1C5C 00001C5C 040C0924 */  addiu      t1, zero, 0xC04
    /* 1C60 00001C60 FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1C64 00001C64 300129AC */  sw         t1, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1C68 00001C68 01000C3C */  lui        t4, (0x10000 >> 16)
    /* 1C6C 00001C6C 00608C40 */  mtc0       t4, $12 /* handwritten instruction */
    /* 1C70 00001C70 00000000 */  nop
    /* 1C74 00001C74 00000000 */  nop
    /* 1C78 00001C78 00000A24 */  addiu      t2, zero, 0x0
    /* 1C7C 00001C7C 800F0B24 */  addiu      t3, zero, 0xF80
  .L00001C80:
    /* 1C80 00001C80 000040AD */  sw         zero, 0x0(t2)
    /* 1C84 00001C84 100040AD */  sw         zero, 0x10(t2)
    /* 1C88 00001C88 200040AD */  sw         zero, 0x20(t2)
    /* 1C8C 00001C8C 300040AD */  sw         zero, 0x30(t2)
    /* 1C90 00001C90 400040AD */  sw         zero, 0x40(t2)
    /* 1C94 00001C94 500040AD */  sw         zero, 0x50(t2)
    /* 1C98 00001C98 600040AD */  sw         zero, 0x60(t2)
    /* 1C9C 00001C9C 700040AD */  sw         zero, 0x70(t2)
    /* 1CA0 00001CA0 F7FF4B15 */  bne        t2, t3, .L00001C80
    /* 1CA4 00001CA4 80004A21 */   addi      t2, t2, 0x80 /* handwritten instruction */
    /* 1CA8 00001CA8 00608040 */  mtc0       zero, $12 /* handwritten instruction */
    /* 1CAC 00001CAC 00000000 */  nop
    /* 1CB0 00001CB0 FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1CB4 00001CB4 30012DAC */  sw         t5, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1CB8 00001CB8 FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1CBC 00001CBC 3001208C */  lw         zero, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1CC0 00001CC0 00000000 */  nop
    /* 1CC4 00001CC4 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1CC8 00001CC8 78152FAC */  sw         t7, %lo(D_BF801578)(AT)
    /* 1CCC 00001CCC 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1CD0 00001CD0 7815208C */  lw         zero, %lo(D_BF801578)(AT)
    /* 1CD4 00001CD4 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1CD8 00001CD8 50142EAC */  sw         t6, %lo(D_BF801450)(AT)
    /* 1CDC 00001CDC 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1CE0 00001CE0 5014208C */  lw         zero, %lo(D_BF801450)(AT)
    /* 1CE4 00001CE4 00608840 */  mtc0       t0, $12 /* handwritten instruction */
    /* 1CE8 00001CE8 00000000 */  nop
    /* 1CEC 00001CEC 0800E003 */  jr         ra
    /* 1CF0 00001CF0 00000000 */  nop

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
    /* 1D10 00001D10 00608040 */   mtc0      zero, $12 /* handwritten instruction */
    /* 1D14 00001D14 00000000 */  nop
    /* 1D18 00001D18 00000000 */  nop
    /* 1D1C 00001D1C 80BF0E3C */  lui        t6, %hi(D_BF801450)
    /* 1D20 00001D20 5014CE8D */  lw         t6, %lo(D_BF801450)(t6)
    /* 1D24 00001D24 00000000 */  nop
    /* 1D28 00001D28 FEFF0F24 */  addiu      t7, zero, -0x2
    /* 1D2C 00001D2C 2448CF01 */  and        t1, t6, t7
    /* 1D30 00001D30 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1D34 00001D34 501429AC */  sw         t1, %lo(D_BF801450)(AT)
    /* 1D38 00001D38 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1D3C 00001D3C 5014208C */  lw         zero, %lo(D_BF801450)(AT)
    /* 1D40 00001D40 80BF0F3C */  lui        t7, %hi(D_BF801578)
    /* 1D44 00001D44 7815EF8D */  lw         t7, %lo(D_BF801578)(t7)
    /* 1D48 00001D48 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1D4C 00001D4C 781520AC */  sw         zero, %lo(D_BF801578)(AT)
    /* 1D50 00001D50 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1D54 00001D54 7815208C */  lw         zero, %lo(D_BF801578)(AT)
    /* 1D58 00001D58 FEFF0D3C */  lui        t5, (0xFFFE0130 >> 16)
    /* 1D5C 00001D5C 3001AD8D */  lw         t5, (0xFFFE0130 & 0xFFFF)(t5)
    /* 1D60 00001D60 C4000924 */  addiu      t1, zero, 0xC4
    /* 1D64 00001D64 FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1D68 00001D68 300129AC */  sw         t1, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1D6C 00001D6C 01000C3C */  lui        t4, (0x10000 >> 16)
    /* 1D70 00001D70 00608C40 */  mtc0       t4, $12 /* handwritten instruction */
    /* 1D74 00001D74 00000000 */  nop
    /* 1D78 00001D78 00000000 */  nop
    /* 1D7C 00001D7C 00000A24 */  addiu      t2, zero, 0x0
    /* 1D80 00001D80 80030B24 */  addiu      t3, zero, 0x380
  .L00001D84:
    /* 1D84 00001D84 000040AD */  sw         zero, 0x0(t2)
    /* 1D88 00001D88 100040AD */  sw         zero, 0x10(t2)
    /* 1D8C 00001D8C 200040AD */  sw         zero, 0x20(t2)
    /* 1D90 00001D90 300040AD */  sw         zero, 0x30(t2)
    /* 1D94 00001D94 400040AD */  sw         zero, 0x40(t2)
    /* 1D98 00001D98 500040AD */  sw         zero, 0x50(t2)
    /* 1D9C 00001D9C 600040AD */  sw         zero, 0x60(t2)
    /* 1DA0 00001DA0 700040AD */  sw         zero, 0x70(t2)
    /* 1DA4 00001DA4 F7FF4B15 */  bne        t2, t3, .L00001D84
    /* 1DA8 00001DA8 80004A21 */   addi      t2, t2, 0x80 /* handwritten instruction */
    /* 1DAC 00001DAC 00608040 */  mtc0       zero, $12 /* handwritten instruction */
    /* 1DB0 00001DB0 00000000 */  nop
    /* 1DB4 00001DB4 FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1DB8 00001DB8 30012DAC */  sw         t5, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1DBC 00001DBC FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1DC0 00001DC0 3001208C */  lw         zero, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1DC4 00001DC4 00000000 */  nop
    /* 1DC8 00001DC8 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1DCC 00001DCC 78152FAC */  sw         t7, %lo(D_BF801578)(AT)
    /* 1DD0 00001DD0 80BF013C */  lui        AT, %hi(D_BF801578)
    /* 1DD4 00001DD4 7815208C */  lw         zero, %lo(D_BF801578)(AT)
    /* 1DD8 00001DD8 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1DDC 00001DDC 50142EAC */  sw         t6, %lo(D_BF801450)(AT)
    /* 1DE0 00001DE0 80BF013C */  lui        AT, %hi(D_BF801450)
    /* 1DE4 00001DE4 5014208C */  lw         zero, %lo(D_BF801450)(AT)
    /* 1DE8 00001DE8 00608840 */  mtc0       t0, $12 /* handwritten instruction */
    /* 1DEC 00001DEC 00000000 */  nop
    /* 1DF0 00001DF0 0800E003 */  jr         ra
    /* 1DF4 00001DF4 00000000 */   nop

    .globl func_00001DF8
func_00001DF8:
    mfc0       t0, C0_SR
    nop
    la         t4, func_00001E14
    or         t4, K1BASE
    jr         t4
    // missing nop...

    .globl func_00001E14
func_00001E14:
    /* 1E14 00001E14 00608040 */   mtc0      zero, $12 /* handwritten instruction */
    /* 1E18 00001E18 00000000 */  nop
    /* 1E1C 00001E1C FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1E20 00001E20 300124AC */  sw         a0, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1E24 00001E24 FEFF013C */  lui        AT, (0xFFFE0130 >> 16)
    /* 1E28 00001E28 3001208C */  lw         zero, (0xFFFE0130 & 0xFFFF)(AT)
    /* 1E2C 00001E2C 00000000 */  nop
    /* 1E30 00001E30 00608840 */  mtc0       t0, $12 /* handwritten instruction */
    /* 1E34 00001E34 00000000 */  nop
    /* 1E38 00001E38 0800E003 */  jr         ra
    /* 1E3C 00001E3C 00000000 */   nop

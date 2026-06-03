#ifndef BITS_H_
#define BITS_H_

// clang-format off
#define read8(a)  (*(volatile u_char  *)(a))
#define read16(a) (*(volatile u_short *)(a))
#define read32(a) (*(volatile u_int   *)(a))

#define write8(a, b)  (*(volatile u_char  *)(a) = (b))
#define write16(a, b) (*(volatile u_short *)(a) = (b))
#define write32(a, b) (*(volatile u_int   *)(a) = (b))

#define set8(a, b)  (*(volatile u_char  *)(a) |= (b))
#define set16(a, b) (*(volatile u_short *)(a) |= (b))
#define set32(a, b) (*(volatile u_int   *)(a) |= (b))

#define clear8(a, b)  (*(volatile u_char  *)(a) &= ~(b))
#define clear16(a, b) (*(volatile u_short *)(a) &= ~(b))
#define clear32(a, b) (*(volatile u_int   *)(a) &= ~(b))

// clanc-format on

#endif // BITS_H_

#ifndef DECI2_H_
#define DECI2_H_

#define DECI2_PROTO_DCMP 0x0001
#define DECI2_PROTO_I0TTYP 0x0110
#define DECI2_PROTO_IKTTYP 0x011f
#define DECI2_PROTO_DRFP0 0x0120
#define DECI2_PROTO_DRFP1 0x0121
#define DECI2_PROTO_ISDBGP 0x0130
#define DECI2_PROTO_ILOADP 0x0150
#define DECI2_PROTO_E0TTYP 0x0210
#define DECI2_PROTO_EKTTYP 0x021f
#define DECI2_PROTO_ESDBGP 0x0230

struct sceDeci2Hdr {
	unsigned short len;
	unsigned short rsvd;
	unsigned short proto;
	unsigned char src;
	unsigned char dest;
};

#endif // DECI2_H_

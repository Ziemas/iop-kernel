#ifndef DECI2_INTERNAL_H_
#define DECI2_INTERNAL_H_

#include "sys/types.h"

struct deci2_iface {
	/* 0x0 */ int node;
	/* 0x0 */ int (*handler)();
	/* 0x8 */ void *opt;
	/* 0xc */ int unkC;
	/* 0x10 */ int unk10;
	/* 0x14 */ int unk14;
	/* 0x18 */ int unk18;
	/* 0x1c */ int unk1C;
	/* 0x20 */ int unk20;
	/* 0x24 */ int unk24;
	/* 0x28 */ int unk28;
	/* 0x2c */ void* unk2C;
};

struct deci2_iface *sceDeci2IfCreate(char, void *, int (*)(), int (*)());
void sceDeci2IfEventHandler(int event, struct deci2_iface *iface, int len, int protocol, int node);
void sceDeci2ExPoll();
void sceDeci2DbgPrintStatus(void (*cb)(void *context, int c), void *param);

#endif // DECI2_INTERNAL_H_

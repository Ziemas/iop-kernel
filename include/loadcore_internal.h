#ifndef LOADCORE_INTERNAL_H_
#define LOADCORE_INTERNAL_H_

#include "sys/types.h"

typedef struct _libinfo {
	unsigned long dummy[2];
	unsigned short version;
	unsigned short dummy2;
	char name[8];
} LibInfo;

int AddRebootNotifyHandler(void *func, int priority, int *stat);
void *QueryLibraryEntryTable(LibInfo *libinfo);

// It always seems like the first word is constructed in a register
// so the struct probably can't be used for RegisterBootMode
#define BOOTMODE(key, value, extwords) (((key) << 16) | (value) | ((extwords) << 24))

typedef struct bootparam {
	unsigned short value;
	unsigned char key;
	unsigned char extwords;
	unsigned long eword[0];
} BootParam;

void RegisterBootMode(int *param);
BootParam *QueryBootMode(int key);
#define QueryBootParam(key) QueryBootMode(key)

/* key */
#define KEY_DECItype 1
#define KEY_EEbootparam 2
#define KEY_IOPbootparam 3

/* IOPbootparam */
#define IBP_no_sif_init (1 << 0)
#define IBP_no_iop_service (1 << 1)
#define IBP_no_auto_load (1 << 2)

struct exe_info {
	int type;
	int entry;
	int gp;
	int vaddr;
	int text_size;
	int data_size;
	int bss_size;
	int memsz;
	int modinfo;
};

struct IopMod {
	u_int moduleinfo;
	u_int entry;
	u_int gp_value;
	u_int text_size;
	u_int data_size;
	u_int bss_size;
	u_short moduleversion;
	char modulename[0];
};

#endif // LOADCORE_INTERNAL_H_

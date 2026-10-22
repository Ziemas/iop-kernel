#ifndef COFF_H_
#define COFF_H_

#include "sys/types.h"

struct coff_fhdr {
	u_short f_magic;
	u_short f_nscns;
	int f_timdat;
	int f_symptr;
	int f_nsyms;
	u_short f_opthdr;
	u_short f_flags;
};

struct coff_opthdr {
	short magic;
	short vstamp;
	u_int tsize;
	u_int dsize;
	u_int bsize;
	u_int entry;
	u_int text_start;
	u_int data_start;
	u_int bss_start;
	u_int gprmask;
	u_int cprmask[4];
	u_int gp_value;
};

struct coff_shdr {
	char s_name[8];
	u_int s_paddr;
	u_int s_vaddr;
	u_int s_size;
	u_int s_scnptr;
	u_int s_relptr;
	u_int s_lnnoptr;
	u_int s_nreloc;
	u_int s_nlnno;
	u_int s_flags;
};

#endif // COFF_H_

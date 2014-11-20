#ifndef _UREG_H_
#define _UREG_H_

/* See intel-sys.pdf section 5.12, "Exception and Interrupt Reference" */
#define SWEXN_CAUSE_DIVIDE       0x00  /* Very clever, Intel */
#define SWEXN_CAUSE_DEBUG        0x01
#define SWEXN_CAUSE_BREAKPOINT   0x03
#define SWEXN_CAUSE_OVERFLOW     0x04
#define SWEXN_CAUSE_BOUNDCHECK   0x05
#define SWEXN_CAUSE_OPCODE       0x06  /* SIGILL */
#define SWEXN_CAUSE_NOFPU        0x07  /* FPU missing/disabled/busy */
#define SWEXN_CAUSE_SEGFAULT     0x0B  /* segment not present */
#define SWEXN_CAUSE_STACKFAULT   0x0C  /* ouch */
#define SWEXN_CAUSE_PROTFAULT    0x0D  /* aka GPF */
#define SWEXN_CAUSE_PAGEFAULT    0x0E  /* cr2 is valid! */
#define SWEXN_CAUSE_FPUFAULT     0x10  /* old x87 FPU is angry */
#define SWEXN_CAUSE_ALIGNFAULT   0x11
#define SWEXN_CAUSE_SIMDFAULT    0x13  /* SSE/SSE2 FPU is angry */

/* Used for aswexn P4 F14 */
#define SIGKILL        0x18
#define SIGALRM        0x19
#define SIGVTALRM      0x1A
#define SIGDANGER      0x1B
#define SIGUSR1        0x1C
#define SIGUSR2        0x1D
#define SIGUSR3        0x1E
#define MIN_SIG SIGKILL
#define MAX_SIG SIGUSR3

/* sigaction_t values */
#define ASWEXN_SET 0
#define ASWEXN_ADD 1
#define ASWEXN_SUBTRACT 2

/* atimer() modes */
#define ASWEXN_VIRTUAL  0
#define ASWEXN_REAL     1

#ifndef ASSEMBLER
#include <stdint.h>
typedef uint32_t sigaction_t;
typedef uint32_t sigmask_t;

typedef struct ureg_t {
	unsigned int cause;
	union {
        /* zero if neither field is relevant */
        unsigned int cr2;       /* #PF only */
        int          signaler;  /* ASWEXN P4F14 */
	};
	unsigned int ds;
	unsigned int es;
	unsigned int fs;
	unsigned int gs;

	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int zero;  /* Dummy %esp, set to zero */
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;

	unsigned int error_code;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
} ureg_t;

#endif /* ASSEMBLER */

#endif /* _UREG_H_ */

#include "push_pop_helper.h"

.global halt
.global readfile

.extern sys_halt
.extern sys_readfile

halt:

	PUSHREGS

	call 	sys_halt

	# This function won't return



readfile:

	PUSHREGS

	pushl 	12(%esi)	# offset
	pushl 	8(%esi)		# count
	pushl 	4(%esi)		# buf
	pushl 	(%esi)		# filename
	call 	sys_readfile

	POPREGS

	iret
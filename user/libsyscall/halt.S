#include <syscall_int.h>

.global halt

halt:
INT 	$HALT_INT
ret
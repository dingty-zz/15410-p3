#include <syscall_int.h>

.global gettid

gettid:
INT 	$GETTID_INT
ret
#include <syscall_int.h>

.global get_ticks

get_ticks:
INT 	$GET_TICKS_INT
ret
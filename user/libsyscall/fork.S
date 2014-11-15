#include <syscall_int.h>

.global fork

fork:
INT 	$FORK_INT
ret
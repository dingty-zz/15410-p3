#include <syscall_int.h>

.global getchar

getchar:
INT 	$GETCHAR_INT
ret
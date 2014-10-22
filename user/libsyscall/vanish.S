#include <syscall_int.h>

.global vanish

vanish:
INT 	$VANISH_INT
ret
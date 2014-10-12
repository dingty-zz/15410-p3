.global atomic_xchange

atomic_xchange:
	movl	4(%esp),	%ebx # Address of the lock
	movl 	$1, 		%eax
	xchg 	%eax, 		(%ebx) # Exchange the current lock status with the lock (1)
	ret
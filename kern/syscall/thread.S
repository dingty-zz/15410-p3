

.global gettid

.extern _gettid

gettid:
	pusha
	call _gettid
	popa
	iret
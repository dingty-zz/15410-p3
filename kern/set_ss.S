.global set_ss
set_ss:
	#mov	0x2b, %ss

	pushl 0xfffff
	push 0x2b
	pushf
	pushl 0x1000000   # esp
	pushl 0x23



	iret
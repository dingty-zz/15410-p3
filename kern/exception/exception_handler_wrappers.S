.global PF

PF:
	pushl	%ebp
	movl 	%esp, %ebp

	//start to build the ureg structure
	//considering exceptinos that push error code
	pushl	24(%ebp) //ss
	pushl 	20(%ebp) //esp
	pushl 	16(%ebp) //eflags
	pushl 	12(%ebp) //cs
	pushl 	8(%ebp)  //eip
	pushl 	4(%ebp)  //error code
	pushl	%eax	
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	$0
	pushl	(%ebp)
	pushl	%esi
	pushl	%edi
	pushl	%gs
	pushl	%fs
	pushl	%es
	pushl	%ds
	pushl	$0 		//cr2 location
	pushl	$0x0E

	pushl	%esp	//ureg_t* cur_ureg

	call 	get_real_handler
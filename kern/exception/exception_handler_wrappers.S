.global DE
.global DB
.global BP
.global OF
.global BR
.global UD
.global NM
.global NP
.global SS
.global GP
.global PF
.global MF
.global AC
.global MC
.global XF


// build the ureg structure for exceptions that push error code
// ss,esp,eflags,cs,eip,error code
#define 	PUSH_GENERAL_INFO_1				;\
			pushl	%ebp				    ;\
			movl 	%esp, %ebp			    ;\
			pushl	24(%ebp)        		;\
			pushl 	20(%ebp) 		        ;\
			pushl 	16(%ebp)      			;\
			pushl 	12(%ebp)      			;\
			pushl 	8(%ebp)  				;\
			pushl 	4(%ebp) 			    ;\
			pushl	%eax	 				;\
			pushl	%ecx	 				;\
			pushl	%edx     				;\
			pushl	%ebx				    ;\
			pushl	$0    				    ;\
			pushl	(%ebp) 					;\
			pushl	%esi	 				;\
			pushl	%edi	 				;\
			pushl	%gs	 	 				;\
			pushl	%fs		 				;\
			pushl	%es     				;\
			pushl	%ds      				;\
			pushl	$0 						;

// build the ureg structure for exceptions that do not push error code
// ss,esp,eflags,cs,eip
#define 	PUSH_GENERAL_INFO_2	;\
			pushl	%ebp			    	;\
			movl 	%esp, %ebp		        ;\
			pushl	20(%ebp)       			;\
			pushl 	16(%ebp)      			;\
			pushl 	12(%ebp)      			;\
			pushl 	8(%ebp)            		;\
			pushl 	4(%ebp)            		;\
			pushl 	0     					;\
			pushl	%eax	 				;\
			pushl	%ecx	 				;\
			pushl	%edx     				;\
			pushl	%ebx				    ;\
			pushl	$0    				    ;\
			pushl	(%ebp) 					;\
			pushl	%esi	 				;\
			pushl	%edi	 				;\
			pushl	%gs	 	 				;\
			pushl	%fs		 				;\
			pushl	%es     				;\
			pushl	%ds      				;\
			pushl	$0

//push cur_ureg with type of ureg_t* and call get_real_handler
#define     GO_TO_REAL_HANDLER ;\
			pushl	%esp				;\
			call 	get_real_handler		


DE:
	PUSH_GENERAL_INFO_2
	pushl	$0x00
	GO_TO_REAL_HANDLER

DB:
	PUSH_GENERAL_INFO_2
	pushl	$0x01
	GO_TO_REAL_HANDLER

BP:
	PUSH_GENERAL_INFO_2
	pushl	$0x03
	GO_TO_REAL_HANDLER

OF:
	PUSH_GENERAL_INFO_2
	pushl	$0x04
	GO_TO_REAL_HANDLER

BR:
	PUSH_GENERAL_INFO_2
	pushl	$0x05
	GO_TO_REAL_HANDLER

UD:
	PUSH_GENERAL_INFO_2
	pushl	$0x06
	GO_TO_REAL_HANDLER

NM:
	PUSH_GENERAL_INFO_2
	pushl	$0x07
	GO_TO_REAL_HANDLER

NP:
	PUSH_GENERAL_INFO_1
	pushl	$0x0B
	GO_TO_REAL_HANDLER

SS:
	PUSH_GENERAL_INFO_1
	pushl	$0x0C
	GO_TO_REAL_HANDLER

GP:
	PUSH_GENERAL_INFO_1
	pushl	$0x0D
	GO_TO_REAL_HANDLER

PF:
	PUSH_GENERAL_INFO_1
	pushl	$0x0E
	GO_TO_REAL_HANDLER

MF:
	PUSH_GENERAL_INFO_2
	pushl	$0x10
	GO_TO_REAL_HANDLER

AC:
	PUSH_GENERAL_INFO_1
	pushl	$0x11
	GO_TO_REAL_HANDLER

XF:
	PUSH_GENERAL_INFO_2
	pushl	$0x13
	GO_TO_REAL_HANDLER
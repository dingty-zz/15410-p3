/** @file signal_handler_wrapper.S
 *
 *  @brief 
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
 
.global signal_handler_wrapper
.extern get_real_signal_handler
// build the ureg structure for exceptions that do not push error code
// ss,esp,eflags,cs,eip
#define 	PUSH_GENERAL_INFO	;\
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
			pushl	$0						;\
			pushl   $0

#define 	POP_GENERAL_INFO	;\
			popl    %ds
			popl	%ds						;\
			popl	%ds      				;\
			popl	%es     				;\
			popl	%fs		 				;\
			popl	%gs	 	 				;\
			popl	%edi	 				;\
			popl	%esi	 				;\
			popl	(%ebp) 					;\
			popl	%ebx    				    ;\
			popl	%ebx				    ;\
			popl	%edx     				;\
			popl	%ecx	 				;\
			popl	%eax	 				;\
			popl 	0     					;\
			popl 	4(%ebp)            		;\
			popl 	8(%ebp)            		;\
			popl 	12(%ebp)      			;\
			popl 	16(%ebp)      			;\
			popl	20(%ebp)       			;\
			movl 	%esp, %ebp		        ;\
			popl	%ebp			    	;\

//push cur_ureg with type of ureg_t* and call get_real_handler
#define     GO_TO_REAL_SIGNAL_HANDLER ;\
			pushl	%esp				;\
			call 	get_real_signal_handler	;\
			popl    %esp	


signal_handler_wrapper:
	PUSH_GENERAL_INFO
	pushl	$0			# cause will be set later
	GO_TO_REAL_SIGNAL_HANDLER
	popl	%eax
	POP_GENERAL_INFO

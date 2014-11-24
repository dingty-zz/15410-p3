/** @file aswexn.S
 *
 *  @brief This file initialize the kernel by following the steps specified
 *         in the handout
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
#include "push_pop_helper.h"	
.global asignal
.global await
.global amask
.global atimer

.extern sys_asignal
.extern sys_await
.extern sys_amask
.extern sys_atimer

asignal:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_asignal
	popl 	%esi
	popl 	%esi

	POPREGS
	iret	

await:

	PUSHREGS

	pushl 	%esi
	call 	sys_await
	popl 	%esi
	
	POPREGS

	iret	

amask:

	PUSHREGS

	pushl 	8(%esi)
	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_amask
	popl 	%esi
	popl 	%esi
	popl 	%esi
	
	POPREGS

	iret	

atimer:

	PUSHREGS

	pushl 	4(%esi)
	pushl 	(%esi)
	call 	sys_atimer
	popl 	%esi
	popl 	%esi

	POPREGS
	iret	

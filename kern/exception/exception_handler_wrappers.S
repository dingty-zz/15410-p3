/** @file exception_handler_wrappers.S
 *
 *  @brief This file includes timer handler and keyboard handler wrappers.
 *
 *  @author Xianqi Zeng (xianqiz)
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */
 
.global DE
.global DB
# .global NMI_INTERRUPT
.global BP
.global OF
.global BR
.global UD
.global NM
.global DF
.global TS
.global NP
.global SS
.global GP
.global PF
# .global INTEL_RESERVED
.global MF
.global AC
.global MC
.global XF

.extern _DE
.extern _DB
# .extern NMI_INTERRUPT
.extern _BP
.extern _OF
.extern _BR
.extern _UD
.extern _NM
.extern _DF
.extern _TS
.extern _NP
.extern _SS
.extern _GP
.extern _PF
# .extern INTEL_RESERVED
.extern _MF
.extern _AC
.extern _MC
.extern _XF


DE:

	pusha
    call    _DE
    popa
    iret

DB:

	pusha
    call    _DB
    popa
    iret

BP:

	pusha
    call    _BP
    popa
    iret

OF:

	pusha
    call    _OF
    popa
    iret

BR:

	pusha
    call    _BR
    popa
    iret

UD:

	pusha
    call    _UD
    popa
    iret
    
DF:

	pusha
    call    _DF
    popa
    iret
    
NP:

	pusha
    call    _NP
    popa
    iret
    
GP:

	pusha
    call    _GP
    popa
    iret
    
PF:

	pusha
    call    _PF
    popa
    iret
    
MF:

	pusha
    call    _MF
    popa
    iret

TS:

    pusha
    call    _TS
    popa
    iret

NM:

    pusha
    call    _NM
    popa
    iret

SS:

    pusha
    call    _SS
    popa
    iret
    
AC:

	pusha
    call    _AC
    popa
    iret
    
MC:

	pusha
    call    _MC
    popa
    iret
   
XF:

	pusha
    call    _XF
    popa
    iret
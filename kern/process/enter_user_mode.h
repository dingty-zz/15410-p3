/**
 * @file enter_user_mode.S
 *
 * @brief The file provide function that pops the initial registers from the
 *        kernel stack for a thread and let it enter the user mode by iret
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _ENTER_USER_MODE_H
#define _ENTER_USER_MODE_H

/** @brief Pop all registers from the current kernel stack and
 *         let it enter the user mode by iret
 */
void enter_user_mode(uint32_t edi,
                     uint32_t esi,
                     uint32_t ebp,
                     uint32_t ebx,
                     uint32_t edx,
                     uint32_t ecx,
                     uint32_t eax,
                     uint32_t eip,
                     uint32_t cs,
                     uint32_t eflags,
                     uint32_t esp,
                     uint32_t ss);


#endif /* _ENTER_USER_MODE_H */

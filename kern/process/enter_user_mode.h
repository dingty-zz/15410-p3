/**
 * @file enter_user_mode.h
 *
 * @brief This file defines two status of the spinlock and 
 *          several lock manipulation functions.
 *
 * @author Xianqi Zeng (xianqiz)
 * @author Tianyuan Ding (tding)
 *
 */

#ifndef _ENTER_USER_MODE_H
#define _ENTER_USER_MODE_H

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

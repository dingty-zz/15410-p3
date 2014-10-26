#include <syscall.h>

// the code copied from elsewhere
/*int sys_print(struct trap_state *ts)
{
    if (check_user_space(current->mm, ts->edi, 0)) {
        ts->eax = -1;
        return 0;
    }
    current->state = TASK_UNINTERRUPTABLE;
    char *user_buf = (char *) ts->edi;
    putbytes(user_buf, ts->esi);
    current->state = TASK_RUNNABLE;
//    lprintf_kern("print: %s", user_buf);
    ts->eax = 0;
    return 0;
}

int sys_set_term_color(struct trap_state *ts)
{
    set_term_color(ts->esi);
    ts->eax = 0;
    return 0;
}

int sys_set_cursor_pos(struct trap_state *ts)
{
    if (ts->esi >= CONSOLE_HEIGHT || ts->edi >= CONSOLE_WIDTH) {
        ts->eax = -1;
        return 0;
    }
    set_cursor(ts->esi, ts->edi);
    ts->eax = 0;
    return 0;
}

int sys_get_cursor_pos(struct trap_state *ts)
{
    struct task_struct *p = current;
    if (check_user_space(p->mm, ts->esi, 1) || 
        check_user_space(p->mm, ts->edi, 1)) {
        ts->eax = -1;
        return 0;
    }
    get_cursor((int *) ts->esi, (int *) ts->edi);
    ts->eax = -1;
    return 0;
}*/

char _getchar(void)
{
    return -1;
}

int _readline(int size, char *buf)
{
    return -1;
}

int _print(int size, char *buf)
{
    return -1;
}

int _set_term_color(int color)
{
    return -1;
}

int _get_cursor_pos(int *row, int *col)
{
    return -1;
}

int _set_cursor_pos(int row, int col)
{
    return -1;
}

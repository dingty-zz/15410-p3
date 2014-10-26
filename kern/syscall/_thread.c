#include <syscall.h>

int yield(int pid)
{
    return -1;
}

int deschedule(int *flag)
{
    return -1;
}

int make_runnable(int pid)
{
    return -1;
}

int _gettid(void)
{
    // return the tid from the currenspoding entry in tid
    return 5;

}

int sleep(int ticks)
{
    return -1;
}

int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    return 0; /* FALSE, but placates assert() in crt0.c */
}


#ifndef _SYS_ASWEXN_H
#define _SYS_ASWEXN_H
#include "control_block.h"

signal_t *make_signal_node(int sender, int signum);
void signal_handler_wrapper();
#endif /* _SYS_ASWEXN_H */
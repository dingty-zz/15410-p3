
#include "simics.h"
/* Divide error exception handler */
void _DE() {
    lprintf("De");
    return;
}

/* Debug exception handler */
void _DB() {
    lprintf("Db");

    return;
}

/* exception handler */
void _BP() {
    lprintf("bp");

    return;
}

/* exception handler */
void _OF() {
    lprintf("of");

    return;
}

/* exception handler */
void _BR() {
    lprintf("br");

    return;
}

/* exception handler */
void _UD() {
    lprintf("ud");

    return;
}

/* exception handler */
void _NM() {
    lprintf("nm");

    return;
}

/* exception handler */
void _DF() {
    lprintf("double fault hander is called.");
    MAGIC_BREAK;
    return;
}

/* exception handler */
void _TS() {
    lprintf("ts");

    return;
}

/* exception handler */
void _NP() {
    lprintf("np");

    return;
}

/* exception handler */
void _SS() {
    lprintf("ss");

    return;
}

/* exception handler */
void _GP() {
    lprintf("gp");

    return;
}

/* exception handler */
void _PF() {
    lprintf("Page fault!!");
    return;
}

/* exception handler */
void _MF() {
    lprintf("mf");

    return;
}

/* exception handler */
void _AC() {
    lprintf("ac");

    return;
}

/* exception handler */
void _MC() {
    lprintf("mc");

    return;
}

/* exception handler */
void _XF() {
    lprintf("xf");

    return;
}

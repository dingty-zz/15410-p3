/** @file console.c
 *
 *  @brief This file includes console device driver functions. There are two
 *         ways to represent the cursor coordinates: the CRTC one and the 
 *         logical one. The logical one is the actual cursor, which determines
 *         where the character should be put next. The CRTC cursor is used for
 *         displaying the logical cursor on to screen when it's visible.
 *         If the arguments are incorrect, I did nothing and return.
 *
 *  @author Tianyuan Ding (tding)
 *  @bug No known bugs
 */

#include <console.h>
#include <stdio.h>
#include <asm.h>
#include <simics.h>

/* CRTC cursor coordinates */
static int c_row = 0;
static int c_col = 0;

/* Logic cursor coordinates */
static int l_row = 0;
static int l_col = 0;

/* If CRTC cursor is visible */
static int visible = 0;

/* Terminal color */
static int c_color;

/* Helper functions */
static void scroll_up();
static void wrap();

int putbyte(char ch)
{   /* If ch is new line char */
    if (ch == '\n') wrap();
    /* If ch is a carrage return char */
    else if (ch == '\r') {
        l_col = 0;
        if (visible) c_col = 0;
    }
    /* If ch is backspace */
    else if (ch == '\b') {
        if (l_col != 0) {  // Clear the previous char
            *(char *)(CONSOLE_MEM_BASE + 2 * 
                (CONSOLE_WIDTH * l_row + l_col - 1)) = ' ';
            *(char *)(CONSOLE_MEM_BASE + 2 * 
                (CONSOLE_WIDTH * l_row + l_col - 1 ) + 1) 
            = BGND_BLACK | FGND_WHITE;
            l_col--;
            if (visible) c_col--;
        } // Note I did nothing when a backspace is at the beginning of a line.
    }
    else { // If ch is a normal char
        draw_char(l_row, l_col, ch, c_color);
        if (l_col + 1 == CONSOLE_WIDTH) wrap();
        else {
            l_col++;
            if (visible) c_col++;
        }
    }
    set_cursor(l_row, l_col);
    return ch;
}

void putbytes(const char *s, int len)
{
    if (len <= 0 || s == NULL) return;
    int i;
    for (i = 0; i < len; ++i)
        putbyte(s[i]);
}

int sys_set_term_color(int color)
{
    if (color < 0) return -1;
    c_color = color;
    return color;
}

void get_term_color(int *color)
{
    if (color == NULL) return;
    *color = c_color;
}

int set_cursor(int row, int col)
{
    if ( row >= CONSOLE_HEIGHT || row < 0 
      || col >= CONSOLE_WIDTH || col < 0) return -1;

    if (!visible) {
        l_row = row;
        l_col = col;
        return 0;
    }

    int offset = CONSOLE_WIDTH * row + col;
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, offset & 0xFF);
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, (offset >> 8) & 0xFF);
    c_row = row;
    c_col = col;
    l_row = row;
    l_col = col;
    return 0;
}

void get_cursor(int *row, int *col) {
    if (row == NULL || col == NULL) return;

    *row = l_row;
    *col = l_col;
}

void hide_cursor()
{
    if (!visible) return;

    int offset = CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1) + CONSOLE_WIDTH;
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, offset & 0xFF);
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, (offset >> 8) & 0xFF);
    visible = 0;
}

void show_cursor()
{
    if (visible) return;

    int offset = CONSOLE_WIDTH * l_row + l_col;
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, offset & 0xFF);
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, (offset >> 8) & 0xFF);
    c_row = l_row;
    c_col = l_col;
    visible = 1;
}

void clear_console()
{
    int i, j;
    for (i = 0; i < CONSOLE_HEIGHT; ++i)
        for (j = 0; j < CONSOLE_WIDTH; ++j)
            draw_char(i, j, '\0', BGND_BLACK | FGND_WHITE);

    set_cursor(0, 0);
}


void draw_char(int row, int col, int ch, int color)
{
    if ( row >= CONSOLE_HEIGHT || row < 0 || col >= CONSOLE_WIDTH || col < 0)
        return;
 
    *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col)) = ch;
    *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col) + 1) = color;
}

char get_char(int row, int col)
{
    return *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col));
}

/** @brief Change the cursor to the beginning of the next line.
 *
 *  It will set logical cursor, and CRTC cursor, if visible, to the the
 *  beginning of the next line when called. Scroll up if the current cursor 
 *  is on the bottom line before ready to wrap.
 *
 *  @return void
 **/
static void wrap()
{
    if (l_row + 1 == CONSOLE_HEIGHT) {
        scroll_up();
        l_col = 0;
        if (visible) c_col = 0;
    } else {
        l_col = 0;
        l_row++;
        if (visible) {
            c_col = 0;
            c_row++;
        }
    }
}

/** @brief Scroll up the entire screen and continue to print on the new line.
 *  @return void
 **/
static void scroll_up()
{
    int row, col;
    char ch;
    int color;
    for (row = 1; row < CONSOLE_HEIGHT; ++row) {
        // Copy the contents up a row
        for (col = 0; col < CONSOLE_WIDTH; ++col) { 
            ch = *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col));
            color = *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * row + col) + 1);
            *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * (row - 1) + col)) = ch;
            *(char *)(CONSOLE_MEM_BASE + 2 * (CONSOLE_WIDTH * (row - 1) + col) + 1) = color;
        }
    }
    /* Clear the bottom row */
    for (col = 0; col < CONSOLE_WIDTH; ++col)
        draw_char(CONSOLE_HEIGHT - 1, col, '\0', BGND_BLACK | FGND_WHITE);
}
/** @file  console.c
 *  @brief console driver
 *
 *  Implementation of console driver. Code from P1.
 *
 *  @author Jonathan Xianqi Zeng
 *  @bug No known bugs.
 */

#include <asm.h>
#include <ctype.h>
#include <stdio.h>
#include <x86/video_defines.h>
#include <simics.h>
#include "inc/console.h"

 /* cursor information*/
typedef struct{
    int row;            //row index
    int col;            //col index
    int color;          //console color
    uint16_t offset;    //cursor offset from console_mem_base
    int is_visible;     //visible or not
} cursor_t;


//global variable carrying the information of our cursor;
cursor_t myCursor = {0,0,0x7,0xFF,0};


static void printbyte(char ch);
static void update_cursor();


/** @brief The function print a byte on console
 *
 *  @param character to be printed
 *  @return 0
 */
int putbyte(char ch)
{
    //call helper function to print and then update cursor;
    printbyte(ch);
    update_cursor();
    return 0;
}

/** @brief The function print multiple bytes on console
 *
 *  @param the string to be printed and the length to be printed
 *  @return nothing
 */
void putbytes(const char* s, int len)
{
    if (len <= 0) return;
    int i;
    //print each char in the string and then update cursor;
    for(i = 0; i < len; i++)
    {
        if (s[i] == '\0') break;
        printbyte(s[i]);
    }
    update_cursor();
}

/** @brief The function to set term color to the color specified
 *
 *  @param the number of the color that the terminal to be set to
 *  @return 0 on success, -1 on failure
 */
int sys_set_term_color(int color)
{
    //check color valid or not first;
    if (color < 0 || color > 0xFF) return -1;
    else myCursor.color = color;
    return 0;
}

/** @brief The function set the color code into the pointer argument
 *
 *  @param the pointer to integer to be set to the color of current terminal
 *  @return nothing
 */
void get_term_color(int* color)
{
    if (color== NULL) return;
    *color = myCursor.color;
}

/** @brief The function to set cursor to the specified location
 *
 *  @param x,y integers that specifies the location
 *  @return 0 on success, -1 on failure
 */
int set_cursor(int row, int col)
{
    //check row and col valid or not;
    if (row < 0 || row >= CONSOLE_HEIGHT)
        return -1;
    if (col < 0 || col >= CONSOLE_WIDTH)
        return -1;
    myCursor.row = row;
    myCursor.col = col;
    //update the cursor on the screen after setting it in the info structure;
    update_cursor();
    return 0;
}

/** @brief The function to get the current cursor and store in the argument
 *
 *  @param pointers to store the current location
 *  @return nothing
 */
void get_cursor(int* row, int* col)
{
    lprintf("in get cursor");
    if (row == NULL || col == NULL) return;
    lprintf("cursor row: %d", myCursor.row);
    lprintf("cursor col: %d", myCursor.col);
    lprintf("startrow, %x",(unsigned int)row);
    lprintf("startcol, %x",(unsigned int)col);
    *row = myCursor.row;
    *col = myCursor.col;
}

/** @brief hide cursor from screen
 *
 *  @param nothing
 *  @return nothing
 */
void hide_cursor()
{
    //set the visibility info in the cursor info structure
    if (!myCursor.is_visible) return;
    else 
    {
        myCursor.is_visible = 0;
        update_cursor();
    }
}

/** @brief show cursor on screen
 *
 *  @param nothing
 *  @return nothing
 */
void show_cursor()
{
    //set the visibility info in the cursor info structure
    if (myCursor.is_visible) return;
    else 
    {
        myCursor.is_visible = 1;
        update_cursor();
    }
}

/** @brief clear the console
 *
 *  @param nothing
 *  @return nothing
 */
void clear_console()
{
    int i, j;
    char* curPixel;
    //set cursor back to the beginning of the screen first;
    myCursor.row = 0;
    myCursor.col = 0;
    //clean every position on the screen;
    for(i = 0; i < CONSOLE_HEIGHT; i++)
    {
        for(j = 0; j < CONSOLE_WIDTH; j++)
        {
            curPixel = (char*)CONSOLE_MEM_BASE + 2*(i*CONSOLE_WIDTH+j);
            *curPixel = 0x00;
            *(curPixel + 1) = myCursor.color;
        }
    }
    update_cursor();
}

/** @brief the function to draw a char on screen
 *
 *  @param row, column, char, and its color
 *  @return nothing
 */
void draw_char(int row, int col, int ch, int color)
{
    if (row < 0 || row >= CONSOLE_HEIGHT) return;
    if (col < 0 || col >= CONSOLE_WIDTH) return;
    if (color < 0 || color > 0xff) return;
    //calculate the address to put the character;
    char* addr = (char*)CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col);
    //put the value;
    *addr = ch;
    //then put the color;
    *(addr+1) = color;
}

/** @brief the function return char at specific location
 *
 *  @param row, column
 *  @return the character at the specified location
 */
char get_char(int row, int col)
{
    if (row < 0 || row >= CONSOLE_HEIGHT) return 0;
    if (col < 0 || col >= CONSOLE_WIDTH) return 0;
    //calculate address for the cahr data,
    // and then return the value at that address;
    char* addr = (char*)CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col);
    return *(addr);
}


/** Implementation of helper functions **/

/** @brief Helper function to update the offset in the cursor info structure
 *
 *  @param nothing;
 *  @return nothing
 */
void update_offset()
{
    if (!myCursor.is_visible) myCursor.offset = 0xFFFF;
    else myCursor.offset = CONSOLE_WIDTH * myCursor.row + myCursor.col;
}

/** @brief Helper function to push cursor one position back;
 *
 *  @param nothing;
 *  @return nothing
 */
void cursor_back()
{
    if (myCursor.col > 0) myCursor.col --;
    else if (myCursor.col == 0 && myCursor.row == 0) return;
    else if (myCursor.col == 0 && myCursor.row != 0) 
    {
        myCursor.col = CONSOLE_WIDTH - 1;
        myCursor.row = myCursor.row - 1; 
    }
}

/** @brief Helper function to push cursor one position forward;
 *
 *  @param nothing;
 *  @return nothing
 */
void cursor_next()
{
    if (myCursor.col < CONSOLE_WIDTH-1) myCursor.col ++;
    else
    {
        myCursor.col = 0;
        myCursor.row = myCursor.row ++; 
    }
}

/** @brief Helper function to check whether a screen is already full or not;
 *
 *  @param nothing;
 *  @return 1 if full, 0 otherwise;
 */
int is_screen_full()
{
    if (myCursor.row < CONSOLE_HEIGHT) return 0;
    //if if row>=height and col>0, it's for sure full;
    else if (myCursor.col > 0) return 1;
    //when row>=height and col=0, if cursoe is in visible mode, then it's full;
    else
    {
        if (myCursor.is_visible) return 1;
        else return 0;
    }
}

/** @brief Helper function to scroll up
 *
 *  @param nothing
 *  @return nothing
 */
void scroll_up()
{
    int i, j;
    char *curPixel;
    char *nextPixel;
    myCursor.row = myCursor.row - 1;
    //move each element up a row
    for(i = 0; i < CONSOLE_HEIGHT; i++){
        for(j = 0; j < CONSOLE_WIDTH; j++){
            curPixel = (char *)CONSOLE_MEM_BASE + 2*(i*CONSOLE_WIDTH+j);
            nextPixel = curPixel + 2*CONSOLE_WIDTH;
            //if it's already the last line, then clean the line;
            if(i == CONSOLE_HEIGHT - 1)
            {
                *curPixel = 0x00;
                *(curPixel + 1) = myCursor.color;
            } 
            //else, copy the next row to the current row;
            else 
            {
                *curPixel = *nextPixel;
                *(curPixel + 1) = *(nextPixel+1);
            }
        }
    }
}

/** @brief Helper function to show a char on the screen
 *
 *  @param the character to be shown on the screen
 *  @return nothing
 */
void printbyte(char ch)
{
    uint16_t offset;
    switch(ch)
    {
        //If carriage return, then cursor is at the next row;
        case '\n':
            myCursor.row ++;
            myCursor.col = 0;
            break;
        //going back to the beginning of the column;
        case '\r':
            myCursor.col = 0;
            break;
        //If backspace, then put the cursor back one space by calling
        //helper function cursor_back, and then put 0x00 at the current place
        case '\b':
            cursor_back();
            offset = myCursor.row * CONSOLE_WIDTH + myCursor.col;
            *((char*)CONSOLE_MEM_BASE + 2*offset) = 0x00;
            *((char*)CONSOLE_MEM_BASE + 2*offset + 1) = myCursor.color;
            break;
        default:
            offset = myCursor.row * CONSOLE_WIDTH + myCursor.col;
            *((char*)CONSOLE_MEM_BASE + 2*offset) = ch;
            *((char*)CONSOLE_MEM_BASE + 2*offset + 1) = myCursor.color;
            cursor_next();
    }
    if (is_screen_full()) scroll_up();
}

/** @brief Helper function to update cursor
 *
 *  It set the offset by sending the high bits and low bits separately
 *
 *  @param nothing
 *  @return nothing
 */
void update_cursor()
{
    uint8_t  high_bits, low_bits;
    //update the offset first
    update_offset();
    high_bits = (myCursor.offset >> 8) & 0x00FF ;
    low_bits = myCursor.offset & 0x00FF;
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, high_bits);
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, low_bits);
}
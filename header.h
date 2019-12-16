#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <dirent.h>

#include <gccore.h>

#include <sdcard/wiisd_io.h>
#include <ogc/usbstorage.h>
#include <fat.h>

#include <wiiuse/wpad.h>
#include <ogc/pad.h>

#define FIELD_BYTEPOS(templ, field) (u32)&templ.field - (u32)&templ

// the int parameter is an index to the current ui element
typedef void(*ui_event)(int);

typedef struct {
	int x, y;
	int w, h;
} ui_box;

typedef struct {
	char *str;
	short info;
	short more;
} ui_entry;

typedef struct {
	char *title; // text displayed above 
	ui_entry *lines;
	int top; // index of the first visible line
	int cursor; // index of currently selected line
	int visible; // number of visible lines
	int total; // total number of lines in list
} ui_list;

#define ALIGN_LEFT   0
#define ALIGN_CENTRE 1
#define ALIGN_RIGHT  2

typedef struct {
	// position of the element when rendered
	int x, y;
	int xalign;

	// used to determine the next element to select if a direction is pressed
	short up, down, left, right;

	u8 screen; // the screen the element belongs to
	u8 invert; // flag for whether the foreground and background colours switch

	u8 type; // type of value to draw (string or digit)
	void *value; // what to draw

	u32 hl_colour; // background colour of element when highlighted

	ui_event action;
	ui_event onrevert;
	ui_event onarrive;

	char *bonus; // any additional text
} ui_element;

// fs.c

#define TYPE_NONE  0
#define TYPE_FILE  1
#define TYPE_DIR   2
#define TYPE_OTHER 3

int dev_init(void);
const char *dev_type(void);
void dev_close(void);

ui_entry *get_filenames(char *path, int *total);

// gfx.c

#define COLOUR_BKGND  COLOR_BLACK
#define COLOUR_TEXT   COLOR_WHITE
#define COLOUR_HL     COLOR_BLUE
#define COLOUR_BORDER COLOR_GRAY
// error = pale red
#define COLOUR_ERROR  0xA56AA5BF

// size of the libogc console font, in pixels
#define FONT_WIDTH  8
#define FONT_HEIGHT 16

#define TILE_W 32
#define TILE_H 32

void video_init(void);
void dim_screen(void);
void clear_screen(void);

void fill_box(u32 colour, ui_box *box);
void render_border(u32 colour, int thickness, ui_box *frame);
void highlight_box(u32 back, u32 fore, u32 target, int inv, ui_box *box);

void render_tile(int idx, int x, int y, u32 back, u32 fore);
void render_string(char *str, int x, int y, u32 back, u32 fore);
void render_list(ui_list *sel, int widest);

// input.c

int allow_scroll(int t);
int allow_move(int t);

void poll_input(void);
u32 input_any(void);

int input_home(void);
int input_accept(void);
int input_back(void);
int input_goto(void);

int input_increase(void);
int input_decrease(void);
int input_up(void);
int input_down(void);
int input_left(void);
int input_right(void);
int input_dpad(void);

// ipc.c

int method_count(void);
char *get_method(int idx);

int ipcdev_count(void);
char *get_ipcdev(int idx);

// mem.c

int get_addr_range(u32 addr, int var_sz);
int get_invalid_addr_range(u32 addr);

void sync_exec_range(u32 addr);

void refresh_view(u8 *set, u32 top);
void poke_view(u32 addr, int cursor, u8 value);

void file_memory_transfer(u32 addr, s32 size, FILE *f, int mode);
void extract(u32 addr, s32 size, char *name);
void patch(u32 addr, s32 size, char *name);

// Screens
#define SCR_ALL  0
#define SCR_MAIN 1
#define SCR_VIEW 2
#define SCR_IPC  3

// Really it's 3 but whatever
#define N_SCREENS 4

// Element IDs
#define ELEM_MAIN_TITLE     1
#define ELEM_VIEW_TITLE     2
#define ELEM_IOS_TITLE      3

#define ELEM_MAIN_ADDRESS   4
#define ELEM_MAIN_VALUE     5

#define ELEM_VIEW_GOTO      8
#define N_VIEW_GOTO         8

#define ELEM_VIEW_DGTS      0x10
#define N_VIEW_DGTS         128

#define ELEM_MAIN_DGTS      0x90
#define N_MAIN_DGTS         16

#define ELEM_PEEK           0xa0
#define ELEM_EXTRACT        0xa1
#define ELEM_IOS            0xa2
#define ELEM_VIEW           0xa3
#define ELEM_POKE           0xa4
#define ELEM_PATCH          0xa5
#define ELEM_EXECUTE        0xa6

#define ELEM_IPC_ARG_NAMES  0xb0
#define ELEM_IPC_ARG_VALUES 0xb8
#define N_IPC_ARGS          8

#define ELEM_IPC_DGTS       0xc0
#define N_IPC_DGTS          64

#define ELEM_METHOD         0x100
#define ELEM_PROC           0x101
#define ELEM_REBOOT         0x102
#define ELEM_SUBMIT         0x103

#define ELEM_IOS_LABEL      0x104
#define ELEM_IOS_VALUE      0x105
#define ELEM_IOS_RELOAD     0x106

#define N_ELEMS             (ELEM_IOS_RELOAD + 1)

void init_ui(void);
void render_ui(void);

ui_element *current_element(void);

void elem_action(void);
void elem_revert(void);
void move_to_element(int idx);

int digit_select(int dgt, int x, int y);
ui_entry *list_select(ui_list *sel);

char *choose_file(u32 addr, int *quit);

int view(u32 addr);

#endif

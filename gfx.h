#ifndef GFX_H
#define GFX_H

#include <gccore.h>

#define PALE_RED 0xA56AA5BF

u32 *xfb;
GXRModeObj *rmode;

void render_tile(int idx, int x, int y);

void draw_box(u32 find, u32 replace, int inv, int x, int y, int w, int h);

void dim_screen();

void print_digits(u8 *set, int tile);
void print_digits_menu(int idx, int tile, int y_off, int selected, u32 bcolour);
void clear_digits_menu(int idx);

void highlight_options(int opts, u32 colour);

#endif

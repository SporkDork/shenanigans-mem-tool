#ifndef UI_H
#define UI_H

#include <gccore.h>
#include <stdio.h>

#define SCROLL_TIMER if (scr_timer % 5 != 1 || (scr_timer > 1 && scr_timer < 25)) break

typedef struct {
	int top;
	int cursor;
	int hl;
	int max_lines;
	int size;
} ui_selector;

int pad_sz(int sz, int pad);

int allow_action(int scr_timer);

void get_numbers(u8 *set, u32 *addr, u32 *value);
void set_numbers(u8 *set, u32 addr, u32 value);

int scroll_value(int cur, int dir);
int scroll_menu_hori(int cur, int dir);

void scroll_list(ui_selector *select, int scr_timer, int dir);

char *choose_file(u32 addr, int dev_type, int *quit);

int view(u32 addr);

#endif

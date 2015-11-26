#include "ui.h"
#include "gfx.h"
#include "mem.h"
#include "cache.h"
#include "fs.h"

int pad_sz(int sz, int pad) {
	if (sz % pad) sz += (pad - (sz % pad));
	return sz;
}

void get_numbers(u8 *set, u32 *addr, u32 *value) {
	if (!set || (!addr && !value)) return;
	u32 a = 0, v = 0;
	int i, l = 28;
	for (i = 0; i < 8; i++, l -= 4) {
		if (addr) a |= set[i] << l;
		if (value) v |= set[i+8] << l;
	}
	if (addr) *addr = a;
	if (value) *value = v;
}

void set_numbers(u8 *set, u32 addr, u32 value) {
	int i, s = 28;
	for (i = 0; i < 8; i++, s -= 4) {
		set[i] = (u8)((addr & (0xf << s)) >> s);
		set[i+8] = (u8)((value & (0xf << s)) >> s);
	}
}

int allow_action(int scr_timer) {
	if (scr_timer == 1 ||
		(scr_timer > 25 && scr_timer <= 75 && scr_timer % 5 == 1) ||
		(scr_timer > 75 && scr_timer <= 115 && scr_timer % 4 == 1) ||
		(scr_timer > 115 && scr_timer <= 145 && scr_timer % 3 == 1) ||
		(scr_timer > 145 && scr_timer <= 165 && scr_timer % 2 == 1) ||
		scr_timer > 165)
			return 1;
	return 0;
}

int scroll_value(int cur, int dir) {
	draw_box(COLOR_BLACK, 0, 0, ((cur >> 3) + cur + 1) * 32, 224, 32, 32);
	cur += dir;
	if (cur < 0) cur += 16;
	if (cur >= 16) cur -= 16;
	return cur;
}

int scroll_menu_hori(int cur, int dir) {
	int c = cur & 3;
	cur &= ~3;
	c += dir;
	if (c < 0) c += 4;
	if (c >= 4) c -= 4;
	cur |= c;
	return cur;
}

int scroll_cursor_hori(int cursor, u32 *top, u32 bounds, int dir) {
	if (dir < 0 && cursor <= 0) {
		dir = -dir;
		*top -= pad_sz(dir, 8);
		if (*top >= bounds) cursor = 16 - dir;
	}
	else if (dir > 0 && cursor >= 143) {
		*top += pad_sz(dir, 8);
		if (*top <= bounds) cursor = 127 + dir;
	}
	else cursor += dir;
	return cursor;
}

void scroll_list(ui_selector *select, int scr_timer, int dir) {
	if (scr_timer > 1 && scr_timer < 25) return;
	if (scr_timer % 5 != 1) return;
	if (select->cursor + dir < 0) {
		select->cursor = 0;
		select->top = 0;
		select->hl = 0;
		return;
	}
	else if (select->cursor + dir >= select->size) {
		select->cursor = select->size - 1;
		if (select->size <= select->max_lines) {
			select->top = 0;
			select->hl = select->size - 1;
		}
		else {
			select->top = select->size - select->max_lines;
			select->hl = select->max_lines - 1;
		}
	}
	else {
		select->cursor += dir;
		select->hl = select->max_lines / 2;
		if (select->size <= select->max_lines || select->cursor <= select->max_lines / 2) {
			select->top = 0;
			select->hl = select->cursor;
			return;
		}
		else if (select->cursor > select->size - (select->max_lines + 1) / 2) {
			select->top = select->size - select->max_lines;
			select->hl = select->cursor - select->top;
		}
		else {
			select->top = select->cursor - select->max_lines / 2;
		}
	}
	select->top = select->top < 0 ? 0 : select->top;
}

char *choose_file(u32 addr, int dev_type, int *quit) {
	char path[24];
	sprintf(path, "%s:/memory_tool", dev_exts[dev_type-1]);
	
	int n_files = 0;
	char **files = get_filenames(path, &n_files);
	if (!files || !n_files) return 0;
	int width = get_width(files, n_files);

	ui_selector select = {0};
	select.size = n_files;
	select.max_lines = 7;

	char name[60];
	int i, cursor = -1, scr_timer = 0;

	dim_screen();
	
	printf("\x1b[3;16HChoose a file to patch to memory at 0x%08X:", addr);

	while (cursor < 0) {

		draw_box(COLOR_BLACK, 0, 0, 0, 0, 640, 200);

		for (i = select.top; i < select.top + select.max_lines; i++) {
			if (i >= select.size) break;
			memset(name, 0, 50);
			strncpy(name, files[i], 47);

			int len = strlen(files[i]);
			if (len > 50) strcat(name, "...");
			else if (len > 47) strcat(name, files[i] + 47);

			char c = '|';
			if (i == select.top) {
				if (select.top == 0) c = '-';
				else c = '^';
			}
			else if (i == select.size - 1) c = '-';
			else if (i == select.top + select.max_lines - 1) c = 'v';

			printf("\x1b[%d;16H %s %s\x1b[%d;%dH%c", i-select.top+5, i == select.cursor ? "->" : "  ", name, i-select.top+5, width+22, c);
		}
		draw_box(COLOR_BLACK, COLOR_BLUE, 0, 48, (select.hl + 5) * 16, 544, 16);
		
		while (1) {
			
			VIDEO_WaitVSync();
			WPAD_ScanPads();
			
			u32 down = WPAD_ButtonsDown(0);
			u32 held = WPAD_ButtonsHeld(0);
			
			if (down & WPAD_BUTTON_HOME) {
				close_filenames(files, n_files);
				draw_box(COLOR_BLUE, COLOR_BLACK, 0, 48, (select.hl + 5) * 16, 544, 16);
				if (quit) *quit = 1;
				return NULL;
			}
			else if (down & WPAD_BUTTON_B) {
				close_filenames(files, n_files);
				return NULL;
			}
			else if (down & WPAD_BUTTON_A) {
				cursor = select.cursor;
				break;
			}
			if (held & WPAD_BUTTON_UP) {
				scr_timer++;
				draw_box(COLOR_BLUE, COLOR_BLACK, 0, 48, (select.hl + 5) * 16, 544, 16);
				scroll_list(&select, scr_timer, -1);
				break;
			}
			else if (held & WPAD_BUTTON_DOWN) {
				scr_timer++;
				draw_box(COLOR_BLUE, COLOR_BLACK, 0, 48, (select.hl + 5) * 16, 544, 16);
				scroll_list(&select, scr_timer, 1);
				break;
			}
			else scr_timer = 0;
		}
	}
	draw_box(COLOR_BLUE, COLOR_TEAL, 0, 48, (select.hl + 5) * 16, 544, 16);

	char fn[200];
	sprintf(fn, "%s/%s", path, files[cursor]);
	close_filenames(files, n_files);

	return strdup(fn);
}

int go_to_addr(u32 addr, u32 *top) {
	*top = (addr & ~7) - 32;
	return (addr & 7) * 2 + 64;
}

int view(u32 addr) {

	u32 offset = addr, top = 0, old_offset = 0;
	int cursor = go_to_addr(offset, &top);
	int range = get_addr_range(offset, 4), r = range;
	if (!range) return 0;

	u8 set[144] = {0};
	u8 offset_set[16] = {0};
	int i, j, refresh = 1, go_to = 0, offset_cur = 0, menu = 0, menu_cur = 0, selected = 0, scr_timer = 0;

	while (1) {

		u32 start = mem_ranges_start[range-1];
		u32 end = mem_ranges_end[range-1] - 72;
		if (top < start) top = start;
		if (top > end) top = end;

		refresh_view(set, top);
		if (r && !go_to) set_numbers(offset_set, top + cursor / 2, 0);

		if (refresh) {
			VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

			int c = 0, x_off;
			for (i = 0; i < 9; i++) {
				x_off = 48;
				for (j = 0; j < 16; j++, c++) {
					if (j >= 8) x_off = 80;
					render_tile(set[c], j*32 + x_off, i*40 + 40);
					if (c == cursor) draw_box(COLOR_BLACK, COLOR_BLUE, 0, j*32 + x_off, i*40 + 40, 32, 32);
				}
			}
		}

		if (go_to && refresh) {
			dim_screen();
			refresh = 0;
		}

		if (menu) {
			int x_off = cursor & 0xf;
			int y_off = (cursor / 16) * 40 + 92;
			if (go_to) {
				x_off = offset_cur + 4;
				y_off = 452;
			}
			if (cursor >= 64) y_off -= 200;
			print_digits_menu(x_off, menu_cur, y_off, selected, COLOR_TEAL);
		}

		for (i = 0; i < 8; i++)
			render_tile(offset_set[i], i*32 + 192, 416);

		if (!get_addr_range(offset, 4))
			draw_box(COLOR_BLACK, PALE_RED, 1, 192, 416, 256, 32);
		
		if (go_to)
			draw_box(COLOR_BLACK, COLOR_BLUE, 0, offset_cur*32 + 192, 416, 32, 32);

		while (1) {

			VIDEO_WaitVSync();
			WPAD_ScanPads();
			u32 down = WPAD_ButtonsDown(0);
			u32 held = WPAD_ButtonsHeld(0);

			if (down) selected = 0;
			if (down & WPAD_BUTTON_HOME) {
				return 1;
			}
			else if (down & WPAD_BUTTON_B) {
				if (menu) {
					menu = 0;
					refresh = 1;
				}
				else if (go_to) {
					if (!r) {
						offset = old_offset;
						r = get_addr_range(offset, 4);
					}
					go_to = 0;
					refresh = 1;
				}
				else return 0;
				break;
			}
			else if (down & WPAD_BUTTON_A) {
				if (!menu) menu = 1;
				else {
					selected = 1;
					if (go_to) {
						offset_set[offset_cur] = menu_cur;
						get_numbers(offset_set, &offset, NULL);
						r = get_addr_range(offset, 4);
						if (r) {
							cursor = go_to_addr(offset, &top);
							range = r;
						}
					}
					else {
						poke_view(top + cursor / 2, cursor, menu_cur);
					}
				}
				break;
			}
			else if (down & WPAD_BUTTON_2) {
				go_to = !go_to;
				if (go_to) {
					dim_screen();
					get_numbers(offset_set, &old_offset, NULL);
					refresh = 0;
				}
				else {
					if (!r) {
						offset = old_offset;
						r = get_addr_range(offset, 4);
					}
					refresh = 1;
				}
				break;
			}
			if (held & WPAD_BUTTON_PLUS) {
				scr_timer++;
				if (menu || go_to) {
					SCROLL_TIMER;
				}
				else {
					if (!allow_action(scr_timer)) break;
				}
				if (menu) {
					if (!go_to) cursor = scroll_cursor_hori(cursor, &top, end, 1);
					else {
						offset_cur++;
						if (offset_cur > 7) offset_cur = 0;
					}
					refresh = 1;
				}
				else {
					if (!go_to) top += 8;
				}
				break;
			}
			else if (held & WPAD_BUTTON_MINUS) {
				scr_timer++;
				if (menu || go_to) {
					SCROLL_TIMER;
				}
				else {
					if (!allow_action(scr_timer)) break;
				}
				if (menu) {
					if (!go_to) cursor = scroll_cursor_hori(cursor, &top, start, -1);
					else {
						offset_cur--;
						if (offset_cur < 0) offset_cur = 7;
					}
					refresh = 1;
				}
				else {
					if (!go_to) top -= 8;
				}
				break;
			}
			else if (held & WPAD_BUTTON_UP) {
				if (!menu && go_to) break;
				scr_timer++;
				if (menu || go_to) {
					SCROLL_TIMER;
				}
				else {
					if (!allow_action(scr_timer)) break;
				}
				if (menu) {
					menu_cur -= 4;
					if (menu_cur < 0) menu_cur += 16;
				}
				else {
					if (cursor < 16) top -= 8;
					else cursor -= 16;
				}
				break;
			}
			else if (held & WPAD_BUTTON_DOWN) {
				if (!menu && go_to) break;
				scr_timer++;
				if (menu || go_to) {
					SCROLL_TIMER;
				}
				else {
					if (!allow_action(scr_timer)) break;
				}
				if (menu) {
					menu_cur += 4;
					if (menu_cur >= 16) menu_cur -= 16;
				}
				else {
					if (cursor > 127) top += 8;
					else cursor += 16;
				}
				break;
			}
			else if (held & WPAD_BUTTON_LEFT) {
				scr_timer++;
				if (menu || go_to) {
					SCROLL_TIMER;
				}
				else {
					if (!allow_action(scr_timer)) break;
				}
				if (menu) menu_cur = scroll_menu_hori(menu_cur, -1);
				else if (go_to) {
					offset_cur--;
					if (offset_cur < 0) offset_cur = 7;
				}
				else cursor = scroll_cursor_hori(cursor, &top, start, -1);
				break;
			}
			else if (held & WPAD_BUTTON_RIGHT) {
				scr_timer++;
				if (menu || go_to) {
					SCROLL_TIMER;
				}
				else {
					if (!allow_action(scr_timer)) break;
				}
				if (menu) menu_cur = scroll_menu_hori(menu_cur, 1);
				else if (go_to) {
					offset_cur++;
					if (offset_cur > 7) offset_cur = 0;
				}
				else cursor = scroll_cursor_hori(cursor, &top, end, 1);
				break;
			}
			else scr_timer = 0;
		}
	}
	return 0;
}

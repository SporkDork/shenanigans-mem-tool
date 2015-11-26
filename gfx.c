#include "gfx.h"
#include "tiles.h"

void render_tile(int idx, int x, int y) {
	x /= 2;
	if (idx < 0 || idx > 15) return;
	int i, j;
	for (i = 0; i < 32; i++) {
		u32 set = ((u32*)tiles)[idx*32+i];
		for (j = 0; j < 16; j++) {
			int bits = (set >> ((15-j)*2)) & 0x3;
			u32 colour = COLOR_WHITE;
			if (bits == 1 || bits == 2) colour = COLOR_GRAY;
			if (bits == 3) colour = COLOR_BLACK;
			xfb[(y+i)*320 + x+j] = colour;
		}
	}
}

void draw_box(u32 find, u32 replace, int inv, int x, int y, int w, int h) {
	x /= 2;
	w /= 2;
	if (x >= 320 || y >= 480) return;
	if (!find && !replace) return;
	if (x < 0) {
		if (x+w > 0) {
			w += x;
			x = 0;
		}
		else return;
	}
	if (y < 0) {
		if (y+h > 0) {
			h += y;
			y = 0;
		}
		else return;
	}
	if (x+w > 320) w = 320-x;
	if (y+h > 480) h = 480-y;
	u32 colour = 0;
	if (!find) colour = replace;
	if (!replace) colour = find;
	int i, j;
	for (i = y; i < y+h; i++) {
		for (j = x; j < x+w; j++) {
			if (!colour) {
				u32 c = xfb[i*320+j];
				if ((!inv && c == find) || (inv && c != find)) xfb[i*320+j] = replace;
			}
			else xfb[i*320+j] = colour;
		}
	}
}

void dim_screen() {
	int i, j;
	for (i = 0; i < 480; i++) {
		for (j = 0; j < 320; j++) {
			u32 x = xfb[i*320+j];
			u8 c = (u8)(x >> 24);
			c >>= 1;
			x &= ~0xFF00FF00;
			x |= c << 24;
			x |= c << 8;
			xfb[i*320+j] = x;
		}
	}
}

void print_digits(u8 *set, int tile) {
	if (!set) return;
	int off = 48;
	int i;
	for (i = 0; i < 16; i++) {
		if (i >= 8) off = 80;
		render_tile(set[i], off + i*32, 224);
		if (tile == i) draw_box(COLOR_BLACK, COLOR_BLUE, 0, off + i*32, 224, 32, 32);
	}
}

void print_digits_menu(int idx, int tile, int y_off, int selected, u32 bcolour) {
	if (idx < 0 || idx > 15 || y_off < 0 || y_off > 336) return;
	int x_off = 32 + idx * 30;
	draw_box(bcolour, 0, 0, x_off-4, y_off-4, 136, 136);
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			int x = x_off + j*32, y = y_off + i*32;
			render_tile(i*4 + j, x, y);
			if (tile == i*4 + j) {
				if (selected) {
					draw_box(COLOR_BLACK, COLOR_GREEN, 0, x, y, 32, 32);
					draw_box(COLOR_BLUE, COLOR_GREEN, 0, x, y, 32, 32);
				}
				else {
					draw_box(COLOR_BLACK, COLOR_BLUE, 0, x, y, 32, 32);
					draw_box(COLOR_GREEN, COLOR_BLUE, 0, x, y, 32, 32);
				}
			}
		}
	}
}

void clear_digits_menu(int idx) {
	if (idx < 0 || idx > 15) return;
	int off = 32 + idx * 30;
	draw_box(COLOR_BLACK, 0, 0, off, 64, 128, 128);
}

void highlight_options(int opts, u32 colour) {
	int pos[] = {176, 296, 440, 176, 304, 424};
	int width[] = {32, 56, 32, 32, 40, 56};
	draw_box(COLOR_BLACK, colour, 0, pos[opts], opts < 3 ? 336 : 368, width[opts], 16);
}

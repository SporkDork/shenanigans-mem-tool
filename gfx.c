#include "header.h"
#include "tiles.h"

u32 *xfb = NULL;
GXRModeObj *rmode = NULL;

void video_init() {
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

void clear_screen() {
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
}

void dim_screen() {
	int i, j;
	u32 *p = xfb;
	for (i = 0; i < 480; i++) {
		for (j = 0; j < 320; j++) {
			u32 hue  = *p        & 0x00FF00FF;
			u32 luma = (*p >> 1) & 0xFF00FF00;
			*p++ = hue | luma;
		}
	}
}

int fit_to_screen(ui_box *b) {
	b->x /= 2;
	b->w /= 2;

	if (b->x >= 320 || b->y >= 480)
		return 0;

	if (b->x < 0) {
		if (b->x + b->w > 0) {
			b->w += b->x;
			b->x = 0;
		}
		else
			return 0;
	}
	if (b->y < 0) {
		if (b->y + b->h > 0) {
			b->h += b->y;
			b->y = 0;
		}
		else
			return 0;
	}

	if (b->x + b->w > 320) b->w = 320 - b->x;
	if (b->y + b->h > 480) b->h = 480 - b->y;

	return 1;
}

void fill_box(u32 colour, ui_box *box) {
	ui_box rect;
	memcpy(&rect, box, sizeof(ui_box));
	if (!fit_to_screen(&rect))
		return;

	int i, j;
	for (i = rect.y; i < rect.y + rect.h; i++) {
		for (j = rect.x; j < rect.x + rect.w; j++) {
			xfb[i*320+j] = colour;
		}
	}
}

void render_border(u32 colour, int thicc, ui_box *frame) {
	int span = (thicc * 2) + frame->w;

	ui_box box = {frame->x, frame->y, span, thicc};
	fill_box(colour, &box); // top border

	box.y = frame->y + frame->h + thicc;
	fill_box(colour, &box); // bottom border

	box.y = frame->y + thicc;
	box.w = thicc;
	box.h = frame->h;
	fill_box(colour, &box); // left border

	box.x = frame->x + frame->w + thicc;
	fill_box(colour, &box); // right border
}

void highlight_box(u32 back, u32 fore, u32 target, int inv, ui_box *box) {
	ui_box rect;
	memcpy(&rect, box, sizeof(ui_box));
	if (!fit_to_screen(&rect))
		return;

	int i, j;
	for (i = rect.y; i < rect.y + rect.h; i++) {
		for (j = rect.x; j < rect.x + rect.w; j++) {
			u32 *p = &xfb[i*320+j];
			if ((!inv && *p == target) || (inv && *p != target))
				*p = back;
			else if (fore)
				*p = fore;
		}
	}
}

void render_tile(int idx, int x, int y, u32 back, u32 fore) {
	x /= 2;
	if (idx < 0 || idx > 15) {
		ui_box box = {x, y, TILE_W, TILE_H};
		fill_box(back, &box);
		return;
	}

	u8 back_y = (back >> 24);
	u8 fore_y = (fore >> 24);
	u8 back_u = ((back & 0x00FF0000) >> 16);
	u8 fore_u = ((fore & 0x00FF0000) >> 16);
	u8 back_v = (back & 0xFF);
	u8 fore_v = (fore & 0xFF);

	u8 mid_y = (back_y + fore_y) / 2;
	u8 mid_u = (back_u + fore_u) / 2;
	u8 mid_v = (back_v + fore_v) / 2;

	u32 mid = (mid_y << 24) | (mid_u << 16) | (mid_y << 8) | mid_v;

	u32 palette[] = {
		fore, mid, mid, back
	};

	u32 *tile_p = (u32*)&tiles[idx][0];

	int i, j;
	for (i = 0; i < TILE_H; i++) {
		u32 *xfb_p = &xfb[(y+i) * 320 + x];
		for (j = 0; j < TILE_W/2; j++) {
			int shift = (TILE_W-j-1) * 2;
			int c = (*tile_p >> shift) & 3;
			*xfb_p++ = palette[c];
		}
		tile_p++;
	}
}

void render_string(char *str, int x, int y, u32 back, u32 fore) {
	if (!str)
		return;

	int half_x = FONT_WIDTH / 2;
	int half_y = FONT_HEIGHT / 2;
	int col = (x + half_x) / FONT_WIDTH;
	int row = (y + half_y) / FONT_HEIGHT;

	int len = strlen(str);
	if (col + len > 80)
		len = 80 - col;

	printf("\x1b[%d;%dH%s", row, col, str);
	ui_box box = {x, y, len * FONT_WIDTH, len * FONT_HEIGHT};
	highlight_box(back, fore, COLOR_WHITE, 0, &box);
}

int print_bar(int x, int y, int width) {
	char bar[80] = {0};
	memset(bar, '-', width-2);
	printf("\x1b[%d;%dH+%s+", y, x, bar);
	return y+1;
}

int print_line(char *str, int x, int y, int width) {
	char line[80] = {0};
	int len = width-4;
	memset(line, ' ', len);

	// If the string is too long, truncate it and add "..." to indicate that it is a longer string
	strncpy(line, str, len);
	if (strlen(str) > len)
		line[len-3] = line[len-2] = line[len-1] = '.';

	printf("\x1b[%d;%dH| %s |", y, x, line);
	return y+1;
}

void render_list(ui_list *sel, int widest) {
	// +4 = 2 x outer border ('|') + 2 x inner padding (' ')
	int width = widest + 4;
	if (width < 24) width = 24; // because why not
	if (width > 80) width = 80; // 80 is the maximum width in chars for a console row

	// +4 = 1 for the top bar + 1 for the title + 1 for the middle bar + 1 for the bottom bar
	int height = sel->visible + 4;

	int col = (80 - width) / 2; // centre the list box
	int row = (20 - height) / 2; // 20 is the maximum number of visible lines in a console

	row = print_bar(col, row, width);
	row = print_line(sel->title, col, row, width);
	row = print_bar(col, row, width);

	int i;
	for (i = 0; i < sel->visible; i++) {
		int idx = sel->top + i;
		print_line(sel->lines[idx].str, col, row, width);
		if (idx == sel->cursor) {
			ui_box box = {
				(col+1) * FONT_WIDTH,
				row * FONT_HEIGHT,
				(width-2) * FONT_WIDTH,
				FONT_HEIGHT
			};
			highlight_box(COLOR_BLUE, 0, COLOR_BLACK, 0, &box);
		}
		row++;
	}
}

void render_yesno(int x, int y, char *str) {
	char *lines = strdup(str);

	int i, count = 0;
	for (i = 0; i < strlen(lines); i++) {
		if (lines[i] == '\n') {
			lines[i] = 0;
			count++;
		}
	}

	int widest = 0;
	char *p = lines;
	for (i = 0; i < count; i++) {
		if (strlen(p) > widest)
			widest = strlen(p);

		p += strlen(p) + 1;
	}

	//int 
	y = print_bar(x, y, widest);
}

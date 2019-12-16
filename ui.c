#include "header.h"

// Element types
#define TYPE_NONE   0
#define TYPE_DIGIT  1
#define TYPE_STRING 2
#define TYPE_BUTTON 3

ui_element ui_elems[N_ELEMS] = {0};

int cur_screen = SCR_MAIN;
int selected[N_SCREENS] = {0};

ui_element *current_element() {
	return &ui_elems[selected[cur_screen]];
}

void elem_action() {
	int idx = selected[cur_screen];
	ui_event event = ui_elems[idx].action;
	if (event)
		event(idx);
}

void elem_revert() {
	int idx = selected[cur_screen];
	ui_event event = ui_elems[idx].onrevert;
	if (event)
		event(idx);
	else
		cur_screen = SCR_MAIN;
}

void move_to_element(int idx) {
	if (idx == 0)
		return;

	ui_event event = ui_elems[idx].onarrive;
	if (event)
		event(idx);

	// change selection after calling onarrive() so that onarrive() has access to the previous element
	cur_screen = ui_elems[idx].screen;
	selected[cur_screen] = idx;
}

void autolink_elem_row(int base, int len) {
	int i;
	for (i = 0; i < len; i++) {
		int prev = (len + i - 1) % len;
		int next = (len + i + 1) % len;

		ui_elems[base + i].left = base + prev;
		ui_elems[base + i].right = base + next;
	}
}

void autolink_elem_column(int col, int row_len, int col_len) {
	int i;
	for (i = 0; i < col_len; i++) {
		int prev = (col_len + i - 1) % col_len;
		int next = (col_len + i + 1) % col_len;

		int row = i * row_len;
		prev *= row_len;
		next *= row_len;

		ui_elems[row + col].up = prev + col;
		ui_elems[row + col].down = next + col;
	}
}

void autolink_elem_grid(int base, int width, int height) {
	int i;
	for (i = 0; i < height; i++)
		autolink_elem_row(base + i * width, width);

	for (i = 0; i < width; i++)
		autolink_elem_column(base + i, width, height);
}

ui_list list_patch = {0};
ui_list list_method = {0};
ui_list list_process = {0};

void mm_elem_arrive(int idx) {
	char *str = ui_elems[idx].bonus;
	ui_elems[ELEM_MAIN_ADDRESS].value = str;
	ui_elems[ELEM_MAIN_VALUE].value = str + strlen(str);
}

void elem_view_arrive(int idx) {
	int prev = selected[cur_screen];
	if (prev == ELEM_PEEK || prev == ELEM_POKE) {
		ui_elems[idx].left = prev;
		ui_elems[idx].right = ui_elems[prev].left;
	}
	if (prev == ELEM_IOS || prev == ELEM_EXECUTE) {
		ui_elems[idx].right = prev;
		ui_elems[idx].left = ui_elems[prev].right;
	}

	mm_elem_arrive(idx);
}

void init_main_ui() {
	// Main menu!
	ui_elems[ELEM_MAIN_TITLE] = (ui_element) {
		.value = "Shenanigans!", .screen = SCR_MAIN, .type = TYPE_STRING,
		.x = 0, .y = 20, .xalign = ALIGN_CENTRE
	};

	// Function buttons
	ui_elems[ELEM_PEEK] = (ui_element) {
		.value = "Peek", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.down = ELEM_POKE, .left = ELEM_IOS, .right = ELEM_EXTRACT,
		.onarrive = mm_elem_arrive, .bonus = "Address\0",
		.x = 100, .y = 200, .xalign = ALIGN_LEFT
	};
	ui_elems[ELEM_EXTRACT] = (ui_element) {
		.value = "Extract", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.down = ELEM_VIEW, .left = ELEM_PEEK, .right = ELEM_IOS,
		.onarrive = mm_elem_arrive, .bonus = "Address\0Size",
		.x = 0, .y = 150, .xalign = ALIGN_CENTRE
	};
	ui_elems[ELEM_IOS] = (ui_element) {
		.value = "IOS", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.down = ELEM_EXECUTE, .left = ELEM_EXTRACT, .right = ELEM_PEEK,
		.onarrive = mm_elem_arrive, .bonus = "\0",
		.x = 100, .y = 200, .xalign = ALIGN_RIGHT
	};
	ui_elems[ELEM_VIEW] = (ui_element) {
		.value = "View", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.up = ELEM_EXTRACT, .down = ELEM_PATCH, .left = ELEM_PEEK, .right = ELEM_IOS,
		.onarrive = elem_view_arrive, .bonus = "Address\0",
		.x = 0, .y = 250, .xalign = ALIGN_CENTRE
	};
	ui_elems[ELEM_POKE] = (ui_element) {
		.value = "Poke", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.up = ELEM_PEEK, .left = ELEM_EXECUTE, .right = ELEM_PATCH,
		.onarrive = mm_elem_arrive, .bonus = "Address\0Value",
		.x = 100, .y = 300, .xalign = ALIGN_LEFT
	};
	ui_elems[ELEM_PATCH] = (ui_element) {
		.value = "Patch", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.up = ELEM_VIEW, .left = ELEM_POKE, .right = ELEM_EXECUTE,
		.onarrive = mm_elem_arrive, .bonus = "Address\0Size Limit",
		.x = 0, .y = 350, .xalign = ALIGN_CENTRE
	};
	ui_elems[ELEM_EXECUTE] = (ui_element) {
		.value = "Execute", .screen = SCR_MAIN, .type = TYPE_BUTTON,
		.up = ELEM_IOS, .left = ELEM_PATCH, .right = ELEM_POKE,
		.onarrive = mm_elem_arrive, .bonus = "Address (r3)\0Parameter (r4)",
		.x = 100, .y = 300, .xalign = ALIGN_RIGHT
	};

	// 'list_patch' refers to a menu which lists files in a folder that could be patched to memory.
	// .visible = 7 means that this menu can display up to 7 items at any one time.
	list_patch = (ui_list) { .visible = 7 };

	autolink_elem_row(ELEM_MAIN_DGTS, N_MAIN_DGTS);

	// Set default target below for the main digits to "Peek"
	int i;
	for (i = 0; i < N_MAIN_DGTS; i++)
		ui_elems[ELEM_MAIN_DGTS + i].down = ELEM_PEEK;
}

void init_view_ui() {
	int i;
	for (i = 0; i < N_VIEW_DGTS; i++) {
		ui_elems[ELEM_VIEW_DGTS + i].screen = SCR_VIEW;
		ui_elems[ELEM_VIEW_DGTS + i].type = TYPE_DIGIT;
	}
	for (i = 0; i < N_VIEW_GOTO; i++) {
		ui_elems[ELEM_VIEW_GOTO + i].screen = SCR_VIEW;
		ui_elems[ELEM_VIEW_GOTO + i].type = TYPE_DIGIT;
	}

	autolink_elem_row(ELEM_VIEW_GOTO, N_VIEW_GOTO);
	autolink_elem_grid(ELEM_VIEW_DGTS, 16, N_VIEW_DGTS / 16);
}

char *ipc_arg_names[] = {
	"Method", "Result", "FD", "Arg 0", "Arg 1", "Arg 2", "Arg 3", "Arg 4"
};

void init_ipc_ui() {
	// IPC menu!
	int i;
	for (i = 0; i < N_IPC_ARGS; i++) {
		ui_elems[ELEM_IPC_ARG_NAMES + i] = (ui_element) {
			.value = ipc_arg_names[i], .screen = SCR_IPC, .type = TYPE_STRING
		};
		ui_elems[ELEM_IPC_ARG_VALUES + i] = (ui_element) {
			.value = NULL, .screen = SCR_IPC, .type = TYPE_STRING
		};
	}

	ui_elems[ELEM_SUBMIT] = (ui_element) {
		.value = "Submit", .screen = SCR_IPC, .type = TYPE_STRING
	};
	ui_elems[ELEM_REBOOT] = (ui_element) {
		.value = "Reboot (off)", .screen = SCR_IPC, .type = TYPE_STRING
	};

	int n_methods = method_count();
	ui_entry *methods = malloc(n_methods * sizeof(ui_entry));
	for (i = 0; i < n_methods; i++)
		methods[i] = (ui_entry) { .str = get_method(i), .info = 1 };

	int n_devices = ipcdev_count();
	ui_entry *devices = malloc(n_devices * sizeof(ui_entry));
	for (i = 0; i < n_devices; i++)
		devices[i] = (ui_entry) { .str = get_ipcdev(i), .info = 1 };

	list_method = (ui_list) { .visible = 8, .lines = methods };
	list_process = (ui_list) { .visible = 7, .lines = devices };

	autolink_elem_grid(ELEM_IPC_DGTS, 8, N_IPC_DGTS / 8);
}

void init_ui() {
	init_main_ui();
	init_view_ui();
	init_ipc_ui();
}

void render_ui() {
	int i;
	ui_element *e = &ui_elems[0];
	for (i = 0; i < N_ELEMS; i++) {
		if (!e->type || e->screen != cur_screen)
			continue;

		u32 back = (i == selected[cur_screen]) ? e->hl_colour : COLOUR_BKGND;

		u32 fore;
		if (e->invert) {
			fore = back;
			back = COLOUR_TEXT;
		}
		else
			fore = COLOUR_TEXT;

		if (e->type == TYPE_DIGIT)
			render_tile((int)e->value, e->x, e->y, back, fore);
		else if (e->type == TYPE_STRING || e->type == TYPE_BUTTON)
			render_string((char*)e->value, e->x, e->y, back, fore);

		e++;
	}
}

int widest_line(ui_list *sel) {
	int high = strlen(sel->title);
	int i;
	for (i = 0; i < sel->total; i++) {
		int l = strlen(sel->lines[i].str);
		if (l > high)
			high = l;
	}

	return high;
}

ui_entry *list_select(ui_list *sel) {

	int width = widest_line(sel);
	int scr_timer = 0;
	ui_entry *selection = NULL;

	while (1) {
		VIDEO_WaitVSync();

		render_list(sel, width);
		poll_input();

		if (input_home() || input_back())
			break;

		if (input_accept()) {
			selection = &sel->lines[sel->cursor];
			break;
		}

		if (input_up() || input_down()) {
			if (!allow_move(scr_timer++))
				continue;

			int dir = input_up() ? -1 : 1;

			sel->cursor += dir;
			if (sel->cursor < 0)
				sel->cursor = 0;
			if (sel->cursor >= sel->total)
				sel->cursor = sel->total-1;

			if (sel->cursor < sel->top)
				sel->top = sel->cursor;
			if (sel->cursor >= sel->top + sel->visible)
				sel->top = sel->cursor - sel->visible + 1;

			if (sel->top < 0)
				sel->top = 0;
			if (sel->top > sel->total - sel->visible)
				sel->top = sel->total - sel->visible;
		}
		else
			scr_timer = 0;
	}

	return selection;
}

int choose_exit() {
	return 1;
}

int digit_select(int dgt, int x, int y) {
	u32 outline = COLOUR_BORDER;
	int border = 8;
	int width = TILE_W * 4;
	int height = TILE_H * 4;

	ui_box frame = {x, y, width, height};
	render_border(outline, border, &frame);
	x += border;
	y += border;

	int scroll_timer = 0;
	while (1) {
		VIDEO_WaitVSync();

		int i, j, idx = 0;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				u32 back = idx == dgt ? COLOUR_HL : COLOUR_BKGND;
				render_tile(idx, x + (j*TILE_W), y + (i*TILE_H), back, COLOUR_TEXT);
				idx++;
			}
		}

		poll_input();

		if (input_home()) {
			if (choose_exit()) {
				dgt = -1;
				break;
			}
		}
		if (input_back()) {
			dgt = -1;
			break;
		}

		if (input_accept())
			break;

		if (input_dpad()) {
			if (!allow_move(scroll_timer++))
				continue;

			int col = dgt % 4;
			int row = dgt / 4;

			if (input_up())
				row = (row + 4 - 1) % 4;
			else if (input_down())
				row = (row + 4 + 1) % 4;
			else if (input_left())
				col = (col + 4 - 1) % 4;
			else if (input_right())
				col = (col + 4 + 1) % 4;

			dgt = (row*4) + col;
		}
		else
			scroll_timer = 0;
	}

	return dgt;
}

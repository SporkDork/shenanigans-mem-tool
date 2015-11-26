#include "gfx.h"
#include "ui.h"
#include "fs.h"
#include "mem.h"
#include "cache.h"

int print_u32(u32 addr) {
	printf("\x1b[26;36H%08X", addr);
	return 0;
}

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

int main() {
	video_init();
	WPAD_Init();
	int dev_type = dev_init();
	if (!dev_type)
		printf("\x1b[27;25HCould not find USB or SD card"
		       "\x1b[28;19HMemory patching and extraction won't work");

	char name[50] = {0};

	if (dev_type >= 1 && dev_type <= 2) {
		sprintf(name, "%s:/memory_tool", dev_exts[dev_type-1]);
		if (!exists(name)) mkdir(name, 0777);
		sprintf(name, "%s:/memory_tool/00000000-00000000.bin", dev_exts[dev_type-1]);
	}

	u8 set[16] = {0};
	u32 addr = 0, value = 0;
	int quit = 0, menu = 1, opts = 0, value_cur = 0, menu_cur = 0, selected = 0, scr_timer = 0;

	while (!quit) {

		VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

		print_digits(set, menu == 2 ? -1 : value_cur);
		if (menu == 0) print_digits_menu(value_cur, menu_cur, 64, selected, COLOR_TEAL);

		printf("\x1b[21;22HPeek           Extract           View"
		       "\x1b[23;22HPoke            Patch          Execute");

		if (menu == 2) {
			u32 colour = COLOR_BLUE;
			if (!get_addr_range(addr, 4)) colour = COLOR_RED;
			else {
				if (opts == 0 || opts == 3)
					if (*((u32*)addr) == value) colour = COLOR_GREEN;

				if (opts == 1 || opts == 4) {
					if (dev_type < 1 || dev_type > 2) {
						colour = 0;
						draw_box(COLOR_BLACK, COLOR_GRAY, 1, 290, 154, 68, 60);
					}
					else if (is_file(name)) colour = COLOR_GREEN;
				}
			}
			if (colour) highlight_options(opts, colour);
		}

		while (1) {
			VIDEO_WaitVSync();
			WPAD_ScanPads();
			u32 down = WPAD_ButtonsDown(0);
			if (down) selected = 0;
			if (down & WPAD_BUTTON_HOME) {
				quit = 1;
				break;
			}
			else if (down & WPAD_BUTTON_A) {
				if (menu == 0) { // Set new digit
					set[value_cur] = (u8)menu_cur;
					get_numbers(set, &addr, &value);
					if (dev_type) sprintf(name, "%s:/memory_tool/%08X-%08X.bin", dev_exts[dev_type-1], addr, value);
					selected = 1;
				}
				else if (menu == 1) menu = 0;  // Go to digit select menu
				else if (menu == 2) {          // Do the highlighted action
					get_numbers(set, &addr, &value);
					int range = get_addr_range(addr, 4);
					void (*execute)(u32);
					char *fn = NULL;
					switch (opts) {
						case 0: // Peek value from address
							if (range == 1 || range == 2) sync_before_read((void*)addr, 4);
							value = *((vu32*)addr);
							set_numbers(set, addr, value);
							break;
						case 1: // Copy RAM from <address> of size <value> and save it to a USB or SD card 
							//test_mem2();
							extract(addr, value, name);
							break;
						case 2: // View RAM starting from <address>
							quit = view(addr);
							break;
						case 3: // Poke <value> to <address>
							*((vu32*)addr) = value;
							if (range == 1 || range == 2) sync_after_write((void*)addr, 4);
							break;
						case 4: // Copy a file from a USB or SD card to RAM at <address>. The size is determined by the smallest of <value> or the file's size.
							fn = choose_file(addr, dev_type, &quit);
							if (!fn) break;
							patch(addr, value, fn);
							delete_str(fn);
							break;
						case 5: // Execute code at <address> with <value> as an argument
							sync_exec_range(addr);
							execute = (void(*)(u32))addr;
							execute(value);
							break;
					}
				}
				break;
			}
			else if (down & WPAD_BUTTON_B) {
				if (menu == 0) clear_digits_menu(value_cur);
				menu = 1;
				break;
			}
			u32 held = WPAD_ButtonsHeld(0);
			if (held & WPAD_BUTTON_PLUS) {
				scr_timer++;
				SCROLL_TIMER;
				if (menu == 2) break;
				if (menu == 0) clear_digits_menu(value_cur);
				value_cur = scroll_value(value_cur, 1);
				break;
			}
			else if (held & WPAD_BUTTON_MINUS) {
				scr_timer++;
				SCROLL_TIMER;
				if (menu == 2) break;
				if (menu == 0) clear_digits_menu(value_cur);
				value_cur = scroll_value(value_cur, -1);
				break;
			}
			else if (held & WPAD_BUTTON_UP) {
				scr_timer++;
				SCROLL_TIMER;
				if (menu == 1) break;
				else if (menu == 2) {
					if (opts < 3) menu = 1;
					else opts -= 3;
				}
				else {
					clear_digits_menu(value_cur);
					menu_cur -= 4;
					if (menu_cur < 0) menu_cur += 16;
				}
				break;
			}
			else if (held & WPAD_BUTTON_DOWN) {
				scr_timer++;
				SCROLL_TIMER;
				if (menu == 1) menu = 2;
				else if (menu == 2) {
					if (opts < 3) opts += 3;
				}
				else {
					clear_digits_menu(value_cur);
					menu_cur += 4;
					if (menu_cur >= 16) menu_cur -= 16;
				}
				break;
			}
			else if (held & WPAD_BUTTON_LEFT) {
				scr_timer++;
				SCROLL_TIMER;
				if (menu == 1) value_cur = scroll_value(value_cur, -1);
				else if (menu == 2) {
					if (opts == 0 || opts == 3) opts += 2;
					else opts--;
					if ((dev_type < 1 || dev_type > 2) && (opts == 1 || opts == 4)) opts--;
				}
				else {
					clear_digits_menu(value_cur);
					menu_cur = scroll_menu_hori(menu_cur, -1);
				}
				break;
			}
			else if (held & WPAD_BUTTON_RIGHT) {
				scr_timer++;
				SCROLL_TIMER;
				if (menu == 1) value_cur = scroll_value(value_cur, 1);
				else if (menu == 2) {
					if (opts == 2 || opts == 5) opts -= 2;
					else opts++;
					if ((dev_type < 1 || dev_type > 2) && (opts == 1 || opts == 4)) opts++;
				}
				else {
					clear_digits_menu(value_cur);
					menu_cur = scroll_menu_hori(menu_cur, 1);
				}
				break;
			}
			else scr_timer = 0;
		}
	}
	dev_close(dev_type);
	return 0;
}

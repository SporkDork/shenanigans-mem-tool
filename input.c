#include "header.h"

int allow_scroll(int t) {
	return (t == 1 ||
		(t > 25 && t <= 75 && t % 5 == 1) ||
		(t > 75 && t <= 115 && t % 4 == 1) ||
		(t > 115 && t <= 145 && t % 3 == 1) ||
		(t > 145 && t <= 165 && t % 2 == 1) ||
		t > 165);
}

int allow_move(int t) {
	return (t == 1 || (t > 25 && t % 5 == 1));
}

int input_init = 0;

u32 wpad_down = 0;
u32 wpad_held = 0;
u32 pad_down = 0;
u32 pad_held = 0;

void poll_input(void) {
	if (!input_init) {
		WPAD_Init();
		PAD_Init();
		input_init = 1;
	}

	WPAD_ScanPads();
	PAD_ScanPads();

	wpad_down = WPAD_ButtonsDown(0);
	wpad_held = WPAD_ButtonsHeld(0);
	pad_down = PAD_ButtonsDown(0);
	pad_held = PAD_ButtonsHeld(0);
}

u32 input_any(void) {
	return wpad_down | pad_down;
}

int input_home(void) {
	return (wpad_down & WPAD_BUTTON_HOME) | (pad_down & PAD_BUTTON_MENU);
}

int input_accept(void) {
	return (wpad_down & WPAD_BUTTON_A) | (pad_down & PAD_BUTTON_A);
}

int input_back(void) {
	return (wpad_down & WPAD_BUTTON_B) | (pad_down & PAD_BUTTON_B);
}

int input_goto(void) {
	return (wpad_down & WPAD_BUTTON_2) | (pad_down & PAD_BUTTON_Y);
}

int input_increase(void) {
	return (wpad_held & WPAD_BUTTON_PLUS) | (pad_held & PAD_TRIGGER_R);
}

int input_decrease(void) {
	return (wpad_held & WPAD_BUTTON_MINUS) | (pad_held & PAD_TRIGGER_L);
}

int input_shift(void) {
	return
		(wpad_held & WPAD_BUTTON_PLUS) | (pad_held & PAD_TRIGGER_R) |
		(wpad_held & WPAD_BUTTON_MINUS) | (pad_held & PAD_TRIGGER_L)
	;
}

int input_up(void) {
	return (wpad_held & WPAD_BUTTON_UP) | (pad_held & PAD_BUTTON_UP);
}

int input_down(void) {
	return (wpad_held & WPAD_BUTTON_DOWN) | (pad_held & PAD_BUTTON_DOWN);
}

int input_left(void) {
	return (wpad_held & WPAD_BUTTON_LEFT) | (pad_held & PAD_BUTTON_LEFT);
}

int input_right(void) {
	return (wpad_held & WPAD_BUTTON_RIGHT) | (pad_held & PAD_BUTTON_RIGHT);
}

int input_dpad(void) {
	return
		(wpad_held & WPAD_BUTTON_UP) | (pad_held & PAD_BUTTON_UP) |
		(wpad_held & WPAD_BUTTON_DOWN) | (pad_held & PAD_BUTTON_DOWN) |
		(wpad_held & WPAD_BUTTON_DOWN) | (pad_held & PAD_BUTTON_DOWN) |
		(wpad_held & WPAD_BUTTON_DOWN) | (pad_held & PAD_BUTTON_DOWN)
	;
}

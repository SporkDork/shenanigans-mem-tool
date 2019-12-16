#include "header.h"

int main() {
	video_init();
	init_ui();

	int scroll_timer = 0;
	while (1) {
		VIDEO_WaitVSync();
		render_ui();
		poll_input();

		if (input_home())
			break;

		if (input_accept()) {
			elem_action();
			render_ui();
		}
		else if (input_back()) {
			elem_revert();
			render_ui();
		}

		if (input_dpad()) {
			if (!allow_move(scroll_timer++))
				continue;

			int next = 0;
			ui_element *cur = current_element();

			if (input_up())         next = cur->up;
			else if (input_down())  next = cur->down;
			else if (input_left())  next = cur->left;
			else if (input_right()) next = cur->right;

			move_to_element(next);
			render_ui();
		}
		else
			scroll_timer = 0;
	}
}

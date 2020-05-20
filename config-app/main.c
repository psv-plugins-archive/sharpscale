/*
This file is part of Sharpscale
Copyright 2020 浅倉麗子

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <psp2/ctrl.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/sysmem.h>
#include <vita2d_sys.h>
#include "sharpscale.h"

// do not need newlib heap
int _newlib_heap_size_user = 4 * 1024;
#define CLIB_HEAP_SIZE 256 * 1024

#define BG_COLOUR     0xFFDFDFDF
#define TEXT_BLACK    0xFF202020
#define TEXT_BLUE     0xFFFF8000
#define TEXT_YELLOW   0xFF009090
#define TEXT_RED      0xFF0000FF

static int text_blue(int a) {
	return a ? TEXT_BLUE : TEXT_BLACK;
}

static int text_yellow(int a) {
	return a ? TEXT_YELLOW : TEXT_BLACK;
}

int main(int argc, char **argv) { (void)argc; (void)argv;
	SceUID memid = sceKernelAllocMemBlock(
		"SharpscaleConfigMemBlock",
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		CLIB_HEAP_SIZE,
		NULL);
	void *membase;
	sceKernelGetMemBlockBase(memid, &membase);

	vita2d_clib_pass_mspace(sceClibMspaceCreate(membase, CLIB_HEAP_SIZE));
	vita2d_init();
	vita2d_set_vblank_wait(0);
	vita2d_set_clear_color(BG_COLOUR);

	vita2d_pgf *pgf;
	pgf = vita2d_load_default_pgf();

	void draw_text(int x, int y, int colour, char *txt) {
		vita2d_pgf_draw_text(pgf, x, y, colour, 1.0, txt);
	}

	void draw_text_right(int x, int y, int colour, char *txt) {
		int width = vita2d_pgf_text_width(pgf, 1.0, txt);
		draw_text(960 - width - x, y, colour, txt);
	}

	void draw_text_centre(int y, int colour, char *txt) {
		int width = vita2d_pgf_text_width(pgf, 1.0, txt);
		draw_text((960 - width) / 2, y, colour, txt);
	}

	int ui_row = 0;
	SceCtrlData last_ctrl = {0};
	SharpscaleConfig config = {0};

	bool kmod_linked = *(uint32_t*)SharpscaleGetConfig != 0xE24FC008;
	if (kmod_linked) {
		SharpscaleGetConfig(&config);
	}

	for (;;) {

		SceCtrlData ctrl;
		if (kmod_linked && sceCtrlReadBufferPositive(0, &ctrl, 1) == 1) {
			int btns = ~last_ctrl.buttons & ctrl.buttons;

			if (btns & SCE_CTRL_UP) {
				ui_row = (ui_row - 1 + 4) % 4;

			} else if (btns & SCE_CTRL_DOWN) {
				ui_row = (ui_row + 1) % 4;

			} else if ((btns & SCE_CTRL_LEFT) || (btns & SCE_CTRL_RIGHT)) {
				int inc = (btns & SCE_CTRL_LEFT) ? -1 : 1;

				if (ui_row == 0) {
					config.mode = (config.mode + inc + 4) % 4;
				} else if (ui_row == 1) {
					config.psone_mode = (config.psone_mode + inc + 3) % 3;
				} else if (ui_row == 2) {
					config.bilinear = !config.bilinear;
				} else if (ui_row == 3) {
					config.full_hd = !config.full_hd;
				}

				SharpscaleSetConfig(&config);
			}
			last_ctrl = ctrl;
		}

		vita2d_start_drawing();
		vita2d_clear_screen();

		int line_height = 35;
		int x_pos = 50;
		int y_pos = line_height * 2;

		draw_text(x_pos, y_pos, TEXT_BLACK, "Sharpscale Configuration Menu");
		draw_text_right(x_pos, y_pos, TEXT_BLACK, "© 2020 浅倉麗子");

		vita2d_draw_line(50.0f, line_height * 2.75f, 910.0f, line_height * 2.75f, TEXT_BLACK);

		if (kmod_linked) {
			x_pos = 50;
			y_pos = line_height * 4;
			draw_text(x_pos, y_pos, text_yellow(ui_row == 0), "Scaling mode");
			draw_text(x_pos += 300, y_pos, text_blue(config.mode == SHARPSCALE_MODE_ORIGINAL), "Original");
			draw_text(x_pos += 100, y_pos, text_blue(config.mode == SHARPSCALE_MODE_INTEGER), "Integer");
			draw_text(x_pos += 100, y_pos, text_blue(config.mode == SHARPSCALE_MODE_REAL), "Real");
			draw_text(x_pos += 100, y_pos, text_blue(config.mode == SHARPSCALE_MODE_FITTED), "Fitted");

			x_pos = 50;
			y_pos = line_height * 5;
			draw_text(x_pos, y_pos, text_yellow(ui_row == 1), "PS1 aspect ratio mode");
			draw_text(x_pos += 300, y_pos, text_blue(config.psone_mode == SHARPSCALE_PSONE_MODE_PIXEL), "Pixel");
			draw_text(x_pos += 100, y_pos, text_blue(config.psone_mode == SHARPSCALE_PSONE_MODE_4_3), "4:3");
			draw_text(x_pos += 100, y_pos, text_blue(config.psone_mode == SHARPSCALE_PSONE_MODE_16_9), "16:9");

			x_pos = 50;
			y_pos = line_height * 6;
			draw_text(x_pos, y_pos, text_yellow(ui_row == 2), "Bilinear filtering");
			draw_text(x_pos += 300, y_pos, text_blue(config.bilinear), "On");
			draw_text(x_pos += 100, y_pos, text_blue(!config.bilinear), "Off");

			x_pos = 50;
			y_pos = line_height * 7;
			draw_text(x_pos, y_pos, text_yellow(ui_row == 3), "Enable Full HD");
			draw_text(x_pos += 300, y_pos, text_blue(config.full_hd), "On");
			draw_text(x_pos += 100, y_pos, text_blue(!config.full_hd), "Off");

		} else {
			y_pos = line_height * 4;
			draw_text(x_pos, y_pos, TEXT_RED, "FATAL ERROR");

			y_pos = line_height * 5;
			draw_text(x_pos, y_pos, TEXT_BLACK, "Sharpscale kernel module not loaded or incompatible version.");
		}

		y_pos = line_height * 13;
		draw_text_centre(y_pos, TEXT_BLACK, "CBPS Productions");
		y_pos = line_height * 14;
		draw_text_centre(y_pos, TEXT_BLACK, "forum.devchroma.nl");

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();
	}

	return 0;
}

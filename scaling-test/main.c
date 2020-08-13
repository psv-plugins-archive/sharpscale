/*
This file is part of Sharpscale
Copyright © 2020 浅倉麗子

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
#include <psp2/display.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/dmac.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>

#include <fnblit.h>
#include <psp2dbg.h>

extern char _binary_unifont_sfn_start[];

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000
#define RED   0xFF0000FF
#define BLUE  0xFFFF0000

#define FB_WIDTH 1920
#define FB_HEIGHT 1080
#define FB_LEN (ALIGN(FB_WIDTH, 64) * FB_HEIGHT * 4)

typedef struct {
	int w;
	int h;
} res_t;

#define FB_RES_LEN 7
static res_t fb_res[FB_RES_LEN] = {
	{480, 272},
	{640, 368},
	{720, 408},
	{960, 544},
	{1280, 720},
	{1440, 1080},
	{1920, 1080},
};

static void render(int *fb_base, int width, int pitch, int height) {
	for (int i = 0; i < width/2; i++) {
		for (int j = 0; j < height/2; j++) {
			fb_base[j * pitch + i] = (j % 2 == 0) ? WHITE : BLACK;
		}
	}
	for (int i = width/2; i < width; i++) {
		for (int j = 0; j < height/2; j++) {
			fb_base[j * pitch + i] = (i % 2 == 0) ? WHITE : BLACK;
		}
	}
	for (int i = 0; i < width/2; i++) {
		for (int j = height/2; j < height; j++) {
			fb_base[j * pitch + i] = (i % 2 == 0) ? WHITE : BLACK;
		}
	}
	for (int i = width/2; i < width; i++) {
		for (int j = height/2; j < height; j++) {
			fb_base[j * pitch + i] = (j % 2 == 0) ? WHITE : BLACK;
		}
	}

	int crop = 0;

	if (width == 480 && height == 272) {
		crop = 1;
	} else if (width == 640 && height == 368) {
		crop = 4;
	} else if (width == 960 && height == 544) {
		crop = 2;
	}

	if (crop > 0) {
		for (int i = 0; i < width; i++) {
			fb_base[(crop - 1) * pitch + i] = RED;
			fb_base[(crop - 0) * pitch + i] = BLUE;
			fb_base[(height - crop - 1) * pitch + i] = BLUE;
			fb_base[(height - crop - 0) * pitch + i] = RED;
		}
	}
}

void _start(int args, void *argp) { (void)args; (void)argp;
	SCE_DBG_FILELOG_INIT("ux0:/sharpscale-scaling-test.log");

	fnblit_set_font(_binary_unifont_sfn_start);
	fnblit_set_fg(WHITE);
	fnblit_set_bg(BLACK);

	SceUID fb_mem_id = sceKernelAllocMemBlock(
		"FramebufferMem",
		SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
		ALIGN(FB_LEN, SCE_KERNEL_256KiB),
		NULL);
	if (fb_mem_id < 0) { goto done; }
	int *fb_base;
	sceKernelGetMemBlockBase(fb_mem_id, (void**)&fb_base);

	SceDisplayFrameBuf fb;
	int width, pitch, height;

	void select_res(int idx) {
		// blank the screen first
		sceDisplaySetFrameBuf(NULL, SCE_DISPLAY_SETBUF_NEXTFRAME);
		sceDisplayWaitVblankStartMulti(2);

		width = fb_res[idx].w;
		pitch = ALIGN(width, 64);
		height = fb_res[idx].h;
		render(fb_base, width, pitch, height);

		fnblit_set_fb(fb_base, pitch, width, height);
		fnblit_printf(10, 10, "%dx%d", width, height);
		fnblit_printf(10, height - 42, "Sharpscale Scaling Test");
		fnblit_printf(10, height - 26, "Copyright 2020 浅倉麗子");

		fb = (SceDisplayFrameBuf){sizeof(fb), fb_base, pitch, SCE_DISPLAY_PIXELFORMAT_A8B8G8R8, width, height};
		int ret = sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);
		SCE_DBG_LOG_INFO("Set resolution %dx%d %s\n", width, height, ret == 0 ? "success" : "failed");

		if (ret != 0) {
			sceDmacMemset(fb_base, 0xFF, 960 * 544 * 4);
			fnblit_set_fb(fb_base, 960, 960, 544);
			fnblit_printf(10, 10, "%dx%d failed", width, height);
			fb = (SceDisplayFrameBuf){sizeof(fb), fb_base, 960, SCE_DISPLAY_PIXELFORMAT_A8B8G8R8, 960, 544};
			sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);
		}
	}

	int res_idx = 0;
	select_res(res_idx);

	SceCtrlData last_ctrl;
	sceClibMemset(&last_ctrl, 0x00, sizeof(last_ctrl));

	for (;;) {
		SceCtrlData ctrl;

		if (sceCtrlReadBufferPositive(0, &ctrl, 1) == 1) {
			int btns = ~last_ctrl.buttons & ctrl.buttons;

			if (btns & SCE_CTRL_LEFT) {
				res_idx = (res_idx - 1 < 0) ? FB_RES_LEN - 1 : res_idx - 1;
				select_res(res_idx);
			} else if (btns & SCE_CTRL_RIGHT) {
				res_idx = (res_idx + 1 < FB_RES_LEN) ? res_idx + 1 : 0;
				select_res(res_idx);
			}
		}

		last_ctrl = ctrl;

		sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);
		sceDisplayWaitVblankStartMulti(2);
	}

done:
	SCE_DBG_FILELOG_TERM();
	sceKernelExitProcess(0);
}

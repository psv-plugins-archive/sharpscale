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

#include <arm_neon.h>
#include <string.h>

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/dmac.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>

#include <fnblit.h>
#include <psp2dbg.h>

#include "common.h"

extern char _binary_unifont_sfn_start[];

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
	UNUSED SceUInt32 start_time = sceKernelGetProcessTimeLow();

	int32x2x4_t vert_line = vld4_s32((int32_t[]){WHITE, BLACK, WHITE, BLACK, WHITE, BLACK, WHITE, BLACK});

	for (int j = 0; j < height / 2; j++) {
		memset(fb_base + j * pitch, (j % 2 == 0) ? WHITE : BLACK, width / 2 * 4);

		int i = width / 2;
		while (i < width) {
			vst4_s32(fb_base + j * pitch + i, vert_line);
			i += 8;
		}
	}

	for (int j = height / 2; j < height; j++) {
		int i = 0;
		while (i < width / 2) {
			vst4_s32(fb_base + j * pitch + i, vert_line);
			i += 8;
		}

		memset(fb_base + j * pitch + width / 2, (j % 2 == 0) ? WHITE : BLACK, width / 2 * 4);
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

	SCE_DBG_LOG_DEBUG("Rendered in %d ms\n", (sceKernelGetProcessTimeLow() - start_time) / 1000);
}

void _start(UNUSED int args, UNUSED void *argp) {
	SCE_DBG_FILELOG_INIT("ux0:/sharpscale-scaling-test.log");

	fnblit_set_font(_binary_unifont_sfn_start);
	fnblit_set_fg(WHITE);
	fnblit_set_bg(BLACK);

	SceUID fb_mem_id[2];
	int *fb_base[2];
	int cur_fb_idx = 0;

	for (int i = 0; i < 2; i++) {
		fb_mem_id[i] = sceKernelAllocMemBlock(
			"FramebufferMem",
			SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW,
			ALIGN(FB_LEN, SCE_KERNEL_1MiB),
			NULL);
		GLZ(fb_mem_id[i]);
		sceKernelGetMemBlockBase(fb_mem_id[i], (void**)(fb_base + i));
	}

	SceDisplayFrameBuf fb;
	int width, pitch, height;

	void select_res(int idx) {
		cur_fb_idx = (cur_fb_idx + 1) % 2;

		width = fb_res[idx].w;
		pitch = ALIGN(width, 64);
		height = fb_res[idx].h;
		render(fb_base[cur_fb_idx], width, pitch, height);

		fnblit_set_fb(fb_base[cur_fb_idx], pitch, width, height);
		fnblit_printf(10, 10, "%dx%d", width, height);
		fnblit_printf(10, height - 42, "Sharpscale Scaling Test");
		fnblit_printf(10, height - 26, "Copyright 2020 浅倉麗子");

		fb = (SceDisplayFrameBuf){sizeof(fb), fb_base[cur_fb_idx], pitch, SCE_DISPLAY_PIXELFORMAT_A8B8G8R8, width, height};
		int ret = sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);

		if (ret == 0) {
			SCE_DBG_LOG_INFO("Set resolution %dx%d success\n", width, height);
		} else {
			SCE_DBG_LOG_ERROR("Set resolution %dx%d failed error %08X\n", width, height, ret);

			UNUSED SceUInt32 start_time = sceKernelGetProcessTimeLow();
			sceDmacMemset(fb_base[cur_fb_idx], 0xFF, 960 * 544 * 4);
			SCE_DBG_LOG_DEBUG("Cleared in %d ms\n", (sceKernelGetProcessTimeLow() - start_time) / 1000);

			fnblit_set_fb(fb_base[cur_fb_idx], 960, 960, 544);
			fnblit_printf(10, 10, "%dx%d failed", width, height);
			fb = (SceDisplayFrameBuf){sizeof(fb), fb_base[cur_fb_idx], 960, SCE_DISPLAY_PIXELFORMAT_A8B8G8R8, 960, 544};
		}
	}

	int res_idx = 0;
	select_res(res_idx);

	SceCtrlData last_ctrl = {0};

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
		sceDisplayWaitVblankStart();
	}

fail:
	SCE_DBG_FILELOG_TERM();
	sceKernelExitProcess(0);
}

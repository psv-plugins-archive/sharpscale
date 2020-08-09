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

#include <stdlib.h>
#include <string.h>

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>

#include <fnblit.h>
#include <psp2dbg.h>

#define SFN_FILE_BUF_LEN SCE_KERNEL_1MiB

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

	SceUID font_mem_id = sceKernelAllocMemBlock(
		"FontFileMem",
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		ALIGN(SFN_FILE_BUF_LEN, SCE_KERNEL_4KiB),
		NULL);
	if (font_mem_id < 0) { goto done; }
	void *sfn_file;
	sceKernelGetMemBlockBase(font_mem_id, &sfn_file);

	SceUID sfn_file_fd = sceIoOpen("app0:font.sfn", SCE_O_RDONLY, 0);
	if (sfn_file_fd < 0) { goto done; }

	int bytes_read = sceIoRead(sfn_file_fd, sfn_file, SFN_FILE_BUF_LEN);
	sceIoClose(sfn_file_fd);
	if (bytes_read < 0 || bytes_read == SFN_FILE_BUF_LEN) { goto done; }

	fnblit_set_font(sfn_file);
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

	int width = 0;
	int pitch = 0;
	int height = 0;

	void select_res(int idx) {
		width = fb_res[idx].w;
		pitch = ALIGN(width, 64);
		height = fb_res[idx].h;
		render(fb_base, width, pitch, height);

		fnblit_set_fb(fb_base, pitch, width, height);
		fnblit_printf(10, 10, "%dx%d", width, height);
		fnblit_printf(10, height - 42, "Sharpscale Scaling Test");
		fnblit_printf(10, height - 26, "Copyright 2020 浅倉麗子");

		SCE_DBG_LOG_INFO("Selected resolution %dx%d\n", width, height);
	}

	int res_idx = 0;
	select_res(res_idx);

	SceCtrlData last_ctrl;
	sceClibMemset(&last_ctrl, 0x00, sizeof(last_ctrl));

	for (;;) {
		SceCtrlData ctrl;
		sceClibMemset(&ctrl, 0x00, sizeof(ctrl));

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

		SceDisplayFrameBuf fb = {
			sizeof(fb),
			fb_base,
			pitch,
			SCE_DISPLAY_PIXELFORMAT_A8B8G8R8,
			width,
			height
		};
		sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);
		sceDisplayWaitVblankStartMulti(2);
	}

done:
	sceKernelExitProcess(0);
}

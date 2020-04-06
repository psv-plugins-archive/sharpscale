/*
Sharpscale
Copyright (C) 2020 浅倉麗子

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

#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000

#define FB_WIDTH 960
#define FB_HEIGHT 544
#define FB_LEN (ALIGN(FB_WIDTH, 64) * FB_HEIGHT * 4)

typedef struct {
	int w;
	int h;
} res_t;

#define FB_RES_LEN 4
static res_t fb_res[FB_RES_LEN] = {
	{480, 272},
	{640, 368},
	{720, 408},
	{960, 544},
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
}

int main() {
	SceUID mem_id = sceKernelAllocMemBlock(
		"ScalingTestMemblock",
		SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
		ALIGN(FB_LEN, 0x40000),
		NULL);
	if (mem_id < 0) { goto done; }
	int *fb_base;
	if (sceKernelGetMemBlockBase(mem_id, (void**)&fb_base) < 0) { goto free_mem; }

	int width = 0;
	int pitch = 0;
	int height = 0;

	void select_res(int idx) {
		width = fb_res[idx].w;
		pitch = ALIGN(width, 64);
		height = fb_res[idx].h;
		render(fb_base, width, pitch, height);
		sceClibPrintf("Selected resolution %dx%d\n", width, height);
	}

	int res_idx = 0;
	select_res(res_idx);

	SceCtrlData last_ctrl;
	memset(&last_ctrl, 0x00, sizeof(last_ctrl));

	for (;;) {
		SceCtrlData ctrl;
		memset(&ctrl, 0x00, sizeof(ctrl));

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

free_mem:
	sceKernelFreeMemBlock(mem_id);
done:
	return sceKernelExitProcess(0);
}

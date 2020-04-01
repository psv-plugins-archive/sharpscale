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

#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define FB_WIDTH 960
#define FB_HEIGHT 544
#define FB_LEN (FB_WIDTH * FB_HEIGHT * 4)
#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000

int main() {
	SceUID mem_id = sceKernelAllocMemBlock(
		"ScalingTestMemblock",
		SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
		ALIGN(FB_LEN, 0x40000),
		NULL);
	if (mem_id < 0) { goto done; }
	int *fb_base;
	if (sceKernelGetMemBlockBase(mem_id, (void**)&fb_base) < 0) { goto free_mem; }

	for (int i = 0; i < FB_WIDTH/2; i++) {
		for (int j = 0; j < FB_HEIGHT/2; j++) {
			fb_base[j * FB_WIDTH + i] = (j % 2 == 0) ? WHITE : BLACK;
		}
	}
	for (int i = FB_WIDTH/2; i < FB_WIDTH; i++) {
		for (int j = 0; j < FB_HEIGHT/2; j++) {
			fb_base[j * FB_WIDTH + i] = (i % 2 == 0) ? WHITE : BLACK;
		}
	}
	for (int i = 0; i < FB_WIDTH/2; i++) {
		for (int j = FB_HEIGHT/2; j < FB_HEIGHT; j++) {
			fb_base[j * FB_WIDTH + i] = (i % 2 == 0) ? WHITE : BLACK;
		}
	}
	for (int i = FB_WIDTH/2; i < FB_WIDTH; i++) {
		for (int j = FB_HEIGHT/2; j < FB_HEIGHT; j++) {
			fb_base[j * FB_WIDTH + i] = (j % 2 == 0) ? WHITE : BLACK;
		}
	}

	SceDisplayFrameBuf fb = {
		sizeof(fb),
		fb_base,
		FB_WIDTH,
		SCE_DISPLAY_PIXELFORMAT_A8B8G8R8,
		FB_WIDTH,
		FB_HEIGHT};
	sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);

	for (;;) {
		sceKernelDelayThread(1000*1000);
	}

free_mem:
	sceKernelFreeMemBlock(mem_id);
done:
	return sceKernelExitProcess(0);
}

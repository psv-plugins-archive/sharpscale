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

// Bounty backers: ScHlAuChii, eleriaqueen, mansjg, TG
// Video comparisons: Zodasaur
// SceLowio: xerpi
// Author: 浅倉麗子

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <psp2kern/display.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/lowio/iftu.h>
#include <psp2kern/sblacmgr.h>
#include <taihen.h>
#include "scedisplay.h"
#include "config.h"

#define GLZ(x) do {\
	if ((x) < 0) { goto fail; }\
} while (0)

#define MIN(x, y) ((x) < (y) ? (x) : (y))

__attribute__ ((__format__ (__printf__, 1, 2)))
static void LOG(const char *fmt, ...) {
	(void)fmt;

	#ifdef LOG_PRINTF
	ksceDebugPrintf("\033[0;35m[Sharpscale]\033[0m ");
	va_list args;
	va_start(args, fmt);
	ksceDebugVprintf(fmt, args);
	va_end(args);
	#endif

	#ifdef LOG_FILE
	SceUID fd = ksceIoOpen("ur0:sharpscale.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd) {
		char buf[0x100];
		va_list args;
		va_start(args, fmt);
		ksceIoWrite(fd, buf, vsnprintf(buf, sizeof(buf), fmt, args));
		va_end(args);
		ksceIoClose(fd);
	}
	#endif
}

#define N_HOOK 4
static SceUID hook_id[N_HOOK];
static tai_hook_ref_t hook_ref[N_HOOK];

static SceUID hook_export(int idx, char *mod, int libnid, int funcnid, void *func) {
	hook_id[idx] = taiHookFunctionExportForKernel(KERNEL_PID, hook_ref+idx, mod, libnid, funcnid, func);
	LOG("Hooked %d UID %08X\n", idx, hook_id[idx]);
	return hook_id[idx];
}
#define HOOK_EXPORT(idx, mod, libnid, funcnid, func)\
	hook_export(idx, mod, libnid, funcnid, func##_hook)

static SceUID hook_import(int idx, char *mod, int libnid, int funcnid, void *func) {
	hook_id[idx] = taiHookFunctionImportForKernel(KERNEL_PID, hook_ref+idx, mod, libnid, funcnid, func);
	LOG("Hooked %d UID %08X\n", idx, hook_id[idx]);
	return hook_id[idx];
}
#define HOOK_IMPORT(idx, mod, libnid, funcnid, func)\
	hook_import(idx, mod, libnid, funcnid, func##_hook)

static SceUID hook_offset(int idx, int mod, int ofs, void *func) {
	hook_id[idx] = taiHookFunctionOffsetForKernel(KERNEL_PID, hook_ref+idx, mod, 0, ofs, 1, func);
	LOG("Hooked %d UID %08X\n", idx, hook_id[idx]);
	return hook_id[idx];
}
#define HOOK_OFFSET(idx, mod, ofs, func)\
	hook_offset(idx, mod, ofs, func##_hook)

extern int module_get_offset(SceUID pid, SceUID modid, int segidx, size_t offset, uintptr_t *addr);
#define GET_OFFSET(modid, seg, ofs, addr)\
	module_get_offset(KERNEL_PID, modid, seg, ofs, (uintptr_t*)addr)

static SharpscaleConfig ss_config;

// SceDisplay_8100B000
static SceDisplayHead *head_data = 0;

// SceDisplay_8100B1D4
static int *primary_head_idx = 0;

// protected by mutex at SceDisplay_8100B1E4
static int cur_head_idx = -1;
static int cur_fb_w = 0;
static int cur_fb_h = 0;

// mutex at SceDisplay_8100B1E4 is locked
static int prepare_set_fb_hook(
		int head_idx, int fb_idx, int pitch, int width, int height,
		int paddr, int pixelformat, int flags, int sync_mode) {
	if ((head_idx & ~1) == 0 && (fb_idx & ~1) == 0 && (sync_mode & ~1) == 0) {
		if (head_idx == *primary_head_idx) {
			cur_head_idx = head_idx;
			cur_fb_w = width;
			cur_fb_h = height;
		} else {
			cur_head_idx = -1;
		}
	}
	return TAI_CONTINUE(int, hook_ref[0], head_idx, fb_idx, pitch, width, height,
		paddr, pixelformat, flags, sync_mode);
}

// mutex at SceDisplay_8100B1E4 is locked
static int prepare_fb_compat_hook(
		SceIftuPlaneState *plane, int pitch, int width, int height,
		int unk5, int unk6, int unk7, int unk8, int paddr, int flags) {
	cur_head_idx = *primary_head_idx;
	cur_fb_w = width;
	cur_fb_h = height;
	return TAI_CONTINUE(int, hook_ref[1], plane, pitch, width, height,
		unk5, unk6, unk7, unk8, paddr, flags);
}

// if mutex at SceDisplay_8100B1E4 is not locked, then state points to a zero struct
static int sceIftuSetInputFrameBuffer_hook(int plane, SceIftuPlaneState *state, int bilinear, int sync_mode) {
	if ((cur_head_idx & ~1) != 0 || !state || !state->fb.width || !state->fb.height) {
		goto done;
	}

	int fb_w = cur_fb_w;
	int fb_h = cur_fb_h;
	int head_w = head_data[cur_head_idx].head_w;
	int head_h = head_data[cur_head_idx].head_h;

	int ar_numer = 1;
	int ar_denom = 1;

	if (ss_config.psone_mode != SHARPSCALE_PSONE_MODE_PIXEL
			&& (fb_w < 960 && fb_h < 544)
			&& !(fb_w == 480 && fb_h == 272)
			&& ksceSblACMgrIsPspEmu(SCE_KERNEL_PROCESS_ID_SELF)) {

		if (ss_config.psone_mode == SHARPSCALE_PSONE_MODE_4_3) {
			ar_numer = 4 * fb_h;
			ar_denom = 3 * fb_w;
			fb_w = ar_numer / 3;
		} else if (ss_config.psone_mode == SHARPSCALE_PSONE_MODE_16_9) {
			ar_numer = 16 * fb_h;
			ar_denom = 9 * fb_w;
			fb_w = ar_numer / 9;
		}
	}

	int scale = 0;

	if (ss_config.mode == SHARPSCALE_MODE_INTEGER) {
		scale = MIN(4, MIN(head_w / fb_w, head_h / (fb_h - 16)));
	} else if (ss_config.mode == SHARPSCALE_MODE_REAL) {
		scale = (fb_w <= head_w && (fb_h - 16) <= head_h) ? 1 : 0;
	}

	while (scale > 0 && 0x10000 * ar_denom / (scale * ar_numer) < 0x10000 / 4) {
		scale--;
	}

	if (scale > 0) {
		state->src_w = 0x10000 * ar_denom / (scale * ar_numer);
		state->src_h = 0x10000 / scale;

		fb_w *= scale;
		fb_h *= scale;

		state->src_x = 0;
		state->dst_x = (head_w - fb_w) / 2;

		if (fb_h <= head_h) {
			state->src_y = 0;
			state->dst_y = (head_h - fb_h) / 2;
		} else {
			state->src_y = MIN(0x400 - 1, (fb_h - head_h) * 0x100 / (2 * scale));
			state->dst_y = 0;
		}
	}

	cur_head_idx = -1;

done:
	bilinear = (!ss_config.bilinear && bilinear == 1) ? 0 : bilinear;
	return TAI_CONTINUE(int, hook_ref[2], plane, state, bilinear, sync_mode);
}

static int sceDisplaySetScaleConf_hook(float scale, int head, int index, int mode) {
	if (head == 1) {
		scale = 1.0f;
		mode = 0;
	}
	return TAI_CONTINUE(int, hook_ref[3], scale, head, index, mode);
}

static void startup(void) {
	memset(hook_id, 0xFF, sizeof(hook_id));
	memset(hook_ref, 0xFF, sizeof(hook_ref));
}

static void cleanup(void) {
	for (int i = 0; i < N_HOOK; i++) {
		if (hook_id[i] >= 0) {
			taiHookReleaseForKernel(hook_id[i], hook_ref[i]);
			LOG("Unhooked %d UID %08X\n", i, hook_id[i]);
		}
	}
}

int _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize argc, const void *argv) { (void)argc; (void)argv;
	startup();

	read_config(&ss_config);

	tai_module_info_t minfo;
	minfo.size = sizeof(minfo);
	GLZ(taiGetModuleInfoForKernel(KERNEL_PID, "SceDisplay", &minfo));

	GLZ(GET_OFFSET(minfo.modid, 1, 0x000, &head_data));
	GLZ(GET_OFFSET(minfo.modid, 1, 0x1D4, &primary_head_idx));

	GLZ(HOOK_OFFSET(0, minfo.modid, 0x2CC, prepare_set_fb));
	GLZ(HOOK_OFFSET(1, minfo.modid, 0x004, prepare_fb_compat));
	GLZ(HOOK_IMPORT(2, "SceDisplay", 0xCAFCFE50, 0x7CE0C4DA, sceIftuSetInputFrameBuffer));

	if (ss_config.mode != SHARPSCALE_MODE_ORIGINAL) {
		GLZ(HOOK_EXPORT(3, "SceDisplay", 0x9FED47AC, 0xEB390A76, sceDisplaySetScaleConf));
		LOG("Disable scaling head 1 fb 0 ret %08X\n", ksceDisplaySetScaleConf(1.0f, 1, 0, 0));
		LOG("Disable scaling head 1 fb 1 ret %08X\n", ksceDisplaySetScaleConf(1.0f, 1, 1, 0));
	}

	return SCE_KERNEL_START_SUCCESS;

fail:
	cleanup();
	return SCE_KERNEL_START_FAILED;
}

int module_stop(SceSize argc, const void *argv) { (void)argc; (void)argv;
	cleanup();
	return SCE_KERNEL_STOP_SUCCESS;
}

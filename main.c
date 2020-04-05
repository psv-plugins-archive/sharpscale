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
#include <taihen.h>

#define GLZ(x) do {\
	if ((x) < 0) { goto fail; }\
} while (0)

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

#define N_HOOK 2
static SceUID hook_id[N_HOOK];
static tai_hook_ref_t hook_ref[N_HOOK];

static SceUID hook_export(int idx, char *mod, int libnid, int funcnid, void *func) {
	hook_id[idx] = taiHookFunctionExportForKernel(KERNEL_PID, hook_ref+idx, mod, libnid, funcnid, func);
	LOG("Hooked %d UID %08X\n", idx, hook_id[idx]);
	return hook_id[idx];
}
#define HOOK_EXPORT(idx, mod, libnid, funcnid, func)\
	hook_export(idx, mod, libnid, funcnid, func##_hook)

static int sceIftuSetInputFrameBuffer_hook(int plane, SceIftuPlaneState *state, int bilinear, int sync_mode) {
	if (state->src_w == 0xC000 && state->src_h == 0xC16D) {
		state->src_w = state->src_h = 0x10000;
		state->dst_x = (1280 - 960) / 2;
		state->dst_y = (720 - 544) / 2;
	} else if (state->src_w == 0x8000 && state->src_h == 0x80F3) {
		state->src_h = state->src_w;
	}

	bilinear = (bilinear == 1) ? 0 : bilinear;

	return TAI_CONTINUE(int, hook_ref[0], plane, state, bilinear, sync_mode);
}

static int sceDisplaySetScaleConf_hook(float scale, int head, int index, int mode) {
	if (head == 1) {
		scale = 1.0f;
		mode = 0;
	}
	return TAI_CONTINUE(int, hook_ref[1], scale, head, index, mode);
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

	GLZ(HOOK_EXPORT(0, "SceLowio", 0xCAFCFE50, 0x7CE0C4DA, sceIftuSetInputFrameBuffer));
	GLZ(HOOK_EXPORT(1, "SceDisplay", 0x9FED47AC, 0xEB390A76, sceDisplaySetScaleConf));

	LOG("Disable scaling head 1 fb 0 ret %08X\n", ksceDisplaySetScaleConf(1.0f, 1, 0, 0));
	LOG("Disable scaling head 1 fb 1 ret %08X\n", ksceDisplaySetScaleConf(1.0f, 1, 1, 0));

	return SCE_KERNEL_START_SUCCESS;

fail:
	cleanup();
	return SCE_KERNEL_START_FAILED;
}

int module_stop(SceSize argc, const void *argv) { (void)argc; (void)argv;
	cleanup();
	return SCE_KERNEL_STOP_SUCCESS;
}

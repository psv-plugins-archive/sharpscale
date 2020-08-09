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

#include <string.h>
#include <psp2kern/kernel/iofilemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include "config.h"
#include "sharpscale_internal.h"

#define BASE_PATH     "ur0:/data"
#define SS_BASE_PATH  BASE_PATH"/sharpscale"
#define CONFIG_PATH   SS_BASE_PATH"/config.bin"

SharpscaleConfig ss_config;

static bool is_config_valid(SharpscaleConfig *config) {
	return config->mode < SHARPSCALE_MODE_INVALID
		&& config->psone_ar < SHARPSCALE_PSONE_AR_INVALID
		&& (config->bilinear == true || config->bilinear == false)
		&& (config->unlock_fb_size == true || config->unlock_fb_size == false);
}

int reset_config(SharpscaleConfig *config) {
	config->mode = SHARPSCALE_MODE_INTEGER;
	config->psone_ar = SHARPSCALE_PSONE_AR_4_3;
	config->bilinear = false;
	config->unlock_fb_size = false;
	return 0;
}

int read_config(SharpscaleConfig *config) {
	SceUID fd = ksceIoOpen(CONFIG_PATH, SCE_O_RDONLY, 0);
	if (fd < 0) { goto fail; }

	int ret = ksceIoRead(fd, config, sizeof(*config));
	ksceIoClose(fd);
	if (ret != sizeof(*config)) { goto fail; }

	if (!is_config_valid(config)) { goto fail; }

	return 0;

fail:
	return -1;
}

int write_config(SharpscaleConfig *config) {
	if (!is_config_valid(config)) { goto fail; }

	ksceIoMkdir(BASE_PATH, SCE_STM_RWO);
	ksceIoMkdir(SS_BASE_PATH, SCE_STM_RWO);
	SceUID fd = ksceIoOpen(CONFIG_PATH, SCE_O_WRONLY | SCE_O_CREAT, SCE_STM_RWO);
	if (fd < 0) { goto fail; }

	int ret = ksceIoWrite(fd, config, sizeof(*config));
	ksceIoClose(fd);
	if (ret != sizeof(*config)) { goto fail; }

	return 0;

fail:
	return -1;
}

int SharpscaleGetConfig(SharpscaleConfig *config) {
	if (!is_config_valid(&ss_config)) { goto fail; }
	return ksceKernelMemcpyKernelToUser((uintptr_t)config, &ss_config, sizeof(*config));

fail:
	return -1;
}

int SharpscaleSetConfig(SharpscaleConfig *config) {
	SharpscaleConfig kconfig;
	int ret = ksceKernelMemcpyUserToKernel(&kconfig, (uintptr_t)config, sizeof(kconfig));
	if (ret < 0) { goto fail; }
	if (!is_config_valid(&kconfig)) { goto fail; }
	if (set_unlock_fb_size(kconfig.unlock_fb_size) < 0) { goto fail; }
	memcpy(&ss_config, &kconfig, sizeof(ss_config));
	return write_config(&ss_config);

fail:
	return -1;
}

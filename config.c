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

#include <psp2kern/kernel/error.h>
#include <psp2kern/kernel/iofilemgr.h>
#include <psp2kern/kernel/sysmem.h>

#include <psp2dbg.h>

#include "config.h"
#include "sharpscale_internal.h"

#define BASE_PATH     "ur0:/data"
#define SS_BASE_PATH  BASE_PATH"/sharpscale"
#define CONFIG_PATH   SS_BASE_PATH"/config.bin"

SharpscaleConfig ss_config;

static bool is_config_valid(SharpscaleConfig *config) {
	bool is_valid = config->mode < SHARPSCALE_MODE_INVALID
		&& config->psone_ar < SHARPSCALE_PSONE_AR_INVALID
		&& (config->bilinear == true || config->bilinear == false)
		&& (config->unlock_fb_size == true || config->unlock_fb_size == false);
	if (is_valid) {
		SCE_DBG_LOG_DEBUG("Config is valid\n");
	} else {
		SCE_DBG_LOG_ERROR("Config is invalid\n");
	}
	return is_valid;
}

static int mkdirp(const char *dirname, SceIoMode mode) {
	int ret = ksceIoMkdir(dirname, mode);
	if (ret == 0) {
		SCE_DBG_LOG_INFO("Created directory %s\n", dirname);
	} else if (ret == (int)SCE_ERROR_ERRNO_EEXIST) {
		SCE_DBG_LOG_INFO("Directory %s already exists\n", dirname);
	} else {
		SCE_DBG_LOG_ERROR("Failed to create directory %s error %08X\n", dirname, ret);
	}
	return ret;
}

int reset_config(SharpscaleConfig *config) {
	config->mode = SHARPSCALE_MODE_INTEGER;
	config->psone_ar = SHARPSCALE_PSONE_AR_4_3;
	config->bilinear = false;
	config->unlock_fb_size = false;
	SCE_DBG_LOG_WARNING("Config has been reset\n");
	return 0;
}

int read_config(SharpscaleConfig *config) {
	SceUID fd = ksceIoOpen(CONFIG_PATH, SCE_O_RDONLY, 0);
	if (fd >= 0) {
		SCE_DBG_LOG_INFO("Opened config file %s UID %08X\n", CONFIG_PATH, fd);
	} else {
		SCE_DBG_LOG_ERROR("Failed to open config file %s error %08X\n", CONFIG_PATH, fd);
		goto fail;
	}

	int ret = ksceIoRead(fd, config, sizeof(*config));
	ksceIoClose(fd);
	if (ret == sizeof(*config)) {
		SCE_DBG_LOG_INFO("Read %d bytes\n", ret);
	} else {
		SCE_DBG_LOG_ERROR("Failed to read error %08X\n", ret);
		goto fail;
	}

	if (!is_config_valid(config)) { goto fail; }

	SCE_DBG_LOG_INFO("read_config success\n");
	return 0;

fail:
	SCE_DBG_LOG_ERROR("read_config failed\n");
	return -1;
}

int write_config(SharpscaleConfig *config) {
	if (!is_config_valid(config)) { goto fail; }

	mkdirp(BASE_PATH, SCE_STM_RWO);
	mkdirp(SS_BASE_PATH, SCE_STM_RWO);

	SceUID fd = ksceIoOpen(CONFIG_PATH, SCE_O_WRONLY | SCE_O_CREAT, SCE_STM_RWO);
	if (fd >= 0) {
		SCE_DBG_LOG_INFO("Opened config file %s UID %08X\n", CONFIG_PATH, fd);
	} else {
		SCE_DBG_LOG_ERROR("Failed to open config file %s error %08X\n", CONFIG_PATH, fd);
		goto fail;
	}

	int ret = ksceIoWrite(fd, config, sizeof(*config));
	ksceIoClose(fd);
	if (ret == sizeof(*config)) {
		SCE_DBG_LOG_INFO("Wrote %d bytes\n", ret);
	} else {
		SCE_DBG_LOG_ERROR("Failed to write error %08X\n", ret);
		goto fail;
	}

	SCE_DBG_LOG_INFO("write_config success\n");
	return 0;

fail:
	SCE_DBG_LOG_ERROR("write_config failed\n");
	return -1;
}

int SharpscaleGetConfig(SharpscaleConfig *config) {
	if (!is_config_valid(&ss_config)) { goto fail; }
	if (ksceKernelMemcpyKernelToUser((uintptr_t)config, &ss_config, sizeof(*config)) < 0) { goto fail; }

	SCE_DBG_LOG_DEBUG("SharpscaleGetConfig success\n");
	return 0;

fail:
	SCE_DBG_LOG_ERROR("SharpscaleGetConfig failed\n");
	return -1;
}

int SharpscaleSetConfig(SharpscaleConfig *config) {
	SharpscaleConfig kconfig;
	if (ksceKernelMemcpyUserToKernel(&kconfig, (uintptr_t)config, sizeof(kconfig)) < 0) { goto fail; }
	if (!is_config_valid(&kconfig)) { goto fail; }
	if (set_unlock_fb_size(kconfig.unlock_fb_size) < 0) { goto fail; }
	memcpy(&ss_config, &kconfig, sizeof(ss_config));
	if (write_config(&ss_config) < 0) { goto fail; }

	SCE_DBG_LOG_DEBUG("SharpscaleSetConfig success\n");
	return 0;

fail:
	SCE_DBG_LOG_ERROR("SharpscaleSetConfig failed\n");
	return -1;
}

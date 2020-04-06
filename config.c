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

#include <stdlib.h>
#include <string.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include "config.h"

#define CONFIG_PATH "ur0:/data/sharpscale/config.txt"

void read_config(sharpscale_config_t *config) {
	SceUID fd = ksceIoOpen(CONFIG_PATH, SCE_O_RDONLY, 0);
	if (fd < 0) { goto fail; }

	char buf[4];
	memset(buf, 0x00, sizeof(buf));
	int ret = ksceIoRead(fd, buf, sizeof(buf) - 1);
	ksceIoClose(fd);
	if (ret != sizeof(buf) - 1) { goto fail; }

	config->mode = strtol(buf, NULL, 10);
	config->bilinear = strtol(buf + 2, NULL, 10);

	if (config->mode < 0 || 2 < config->mode) { goto fail; }
	if ((config->bilinear & ~1) != 0) { goto fail; }

	return;

fail:
	config->mode = SHARPSCALE_MODE_INTEGER;
	config->bilinear = 0;
}

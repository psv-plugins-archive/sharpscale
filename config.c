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

#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include "config.h"

void read_config(SharpscaleConfig *config) {
	SceUID fd = ksceIoOpen(CONFIG_PATH, SCE_O_RDONLY, 0);
	if (fd < 0) { goto fail; }

	int ret = ksceIoRead(fd, config, sizeof(*config));
	ksceIoClose(fd);
	if (ret != sizeof(*config)) { goto fail; }

	if (SHARPSCALE_MODE_INVALID <= config->mode) { goto fail; }
	if (SHARPSCALE_PSONE_MODE_INVALID <= config->psone_mode) { goto fail; }
	if (config->bilinear != false && config->bilinear != true) { goto fail; }

	return;

fail:
	config->mode = SHARPSCALE_MODE_INTEGER;
	config->psone_mode = SHARPSCALE_PSONE_MODE_4_3;
	config->bilinear = false;
}

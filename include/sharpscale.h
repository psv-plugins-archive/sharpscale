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

#ifndef SHARPSCALE_H
#define SHARPSCALE_H

#include <stdbool.h>

typedef enum SharpscaleMode {
	SHARPSCALE_MODE_ORIGINAL,
	SHARPSCALE_MODE_INTEGER,
	SHARPSCALE_MODE_REAL,
	SHARPSCALE_MODE_FITTED,
	SHARPSCALE_MODE_INVALID,
} SharpscaleMode;

typedef enum SharpscalePSOneAR {
	SHARPSCALE_PSONE_AR_PIXEL,
	SHARPSCALE_PSONE_AR_4_3,
	SHARPSCALE_PSONE_AR_16_9,
	SHARPSCALE_PSONE_AR_INVALID,
} SharpscalePSOneAR;

typedef struct SharpscaleConfig {
	SharpscaleMode mode;
	SharpscalePSOneAR psone_ar;
	bool bilinear;
	bool unlock_fb_size;
} SharpscaleConfig;

int SharpscaleGetConfig(SharpscaleConfig *config);
int SharpscaleSetConfig(SharpscaleConfig *config);

#endif

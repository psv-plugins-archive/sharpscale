/*
This file is part of Sharpscale
Copyright 2020 浅倉麗子

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

#ifndef SCEDISPLAY_H
#define SCEDISPLAY_H

// From https://wiki.henkaku.xyz/vita/SceDisplay

typedef struct {
	SceUID pid;
	int paddr;
	int scale_mode;
	float scale_ratio;
	int vp_x;
	int vp_y;
	int vp_w;
	int vp_h;
} SceDisplayHeadFrameBuf;

typedef struct {
	int lock;
	int enabled;
	int initialized;
	int vblankcount;
	int cb_event_uid;
	int unk14;
	int pulse_event_value;
	int dsi_bus;
	int plane_idx0;
	int plane_idx1;
	int vic;
	int head_w;
	int head_h;
	int conv_flags;
	int head_pixelformat;
	int pixel_size;
	float refresh_rate;
	int interlaced;
	int brightness;
	int invert_colors;
	SceDisplayHeadFrameBuf fb[2];
	int cb_uid;
	int brightness_control_value;
} SceDisplayHead;

#endif

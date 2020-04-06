#ifndef SCEDISPLAY_H
#define SCEDISPLAY_H

// From https://github.com/vita-nuova/bounties/issues/7#issuecomment-520154064

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

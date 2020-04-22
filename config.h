#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define CONFIG_PATH "ur0:/data/sharpscale/config.bin"

typedef enum SharpscaleMode {
	SHARPSCALE_MODE_ORIGINAL,
	SHARPSCALE_MODE_INTEGER,
	SHARPSCALE_MODE_REAL,
	SHARPSCALE_MODE_INVALID,
} SharpscaleMode;

typedef enum SharpscalePSOneMode {
	SHARPSCALE_PSONE_MODE_PIXEL,
	SHARPSCALE_PSONE_MODE_4_3,
	SHARPSCALE_PSONE_MODE_16_9,
	SHARPSCALE_PSONE_MODE_INVALID,
} SharpscalePSOneMode;

typedef struct SharpscaleConfig {
	SharpscaleMode mode;
	SharpscalePSOneMode psone_mode;
	bool bilinear;
} SharpscaleConfig;

void read_config(SharpscaleConfig *config);

#endif

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

typedef struct SharpscaleConfig {
	SharpscaleMode mode;
	bool bilinear;
} SharpscaleConfig;

void read_config(SharpscaleConfig *config);

#endif

#ifndef CONFIG_H
#define CONFIG_H

#define SHARPSCALE_MODE_ORIGINAL 0
#define SHARPSCALE_MODE_INTEGER  1
#define SHARPSCALE_MODE_REAL     2

typedef struct {
	int mode;
	int bilinear;
} sharpscale_config_t;

void read_config(sharpscale_config_t *config);

#endif

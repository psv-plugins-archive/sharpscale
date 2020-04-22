#ifndef CONFIG_H
#define CONFIG_H

#include "sharpscale.h"

int reset_config(SharpscaleConfig *config);
int read_config(SharpscaleConfig *config);
int write_config(SharpscaleConfig *config);

#endif

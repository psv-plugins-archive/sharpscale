# Sharpscale

Sharpscale is a PSTV and PS Vita plugin that changes the framebuffer to display scaling method to provide a cleaner and sharper image.

Please see [CBPS forums](https://forum.devchroma.nl/index.php/topic,112.0.html) for download, examples, and detailed usage.

## Usage

Sharpscale can be configured to different scaling methods.

#### Scaling modes

- Original: system default
- Integer: framebuffer is scaled by multiplying the width and height by the largest integer such that the resulting width and height is less than or equal to the display width and height, respectively
- Real: no scaling is performed

In integer and real modes, if the framebuffer is already larger than the display, then the framebuffer is scaled down to fit the display, in the same manner as in original mode.

In integer and real modes, up to 16 lines from the bottom may be cropped due to aspect ratio difference between the display resolution and framebuffer resolution.

#### Bilinear filtering

- On: system default
- Off: nearest neighbour

## Installation

Install under `*KERNEL` of your taiHEN config.

```
*KERNEL
ur0:tai/sharpscale.skprx
```

## Configuration

Configuration is provided by a text file at `ur0:/data/sharpscale/config.txt` containing two numbers separated by a space.

1. Scaling mode
	- `0` original
	- `1` integer
	- `2` real
2. Bilinear filtering
	- `0` off
	- `1` on

For example, to use integer mode and turn off bilinear filtering, write `1 0` in the text file.

## Scaling test

The scaling test program shows horizontal and vertical lines 1 pixel wide alternating between black and white. Use the left and right buttons to cycle between framebuffer resolutions.

**Warning:** Do not use when HDMI is set to interlaced mode.

## Credits

- Bounty backers: [ScHlAuChii, eleriaqueen, mansjg, TG](https://www.bountysource.com/issues/78540965-native-resolution-output-for-pstv)
- Video comparisons: Zodasaur
- SceLowio: xerpi
- Author: 浅倉麗子

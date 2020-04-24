# Sharpscale

Sharpscale is a PSTV and PS Vita plugin that changes the framebuffer to display scaling method to provide a cleaner and sharper image.

Please see [CBPS forums](https://forum.devchroma.nl/index.php/topic,112.0.html) for download, examples, and detailed usage.

## Usage

Sharpscale can be configured to different scaling methods.

#### Scaling modes

- Original: system default
- Integer: scale by multiplying the width and height of the framebuffer by the largest integer less than or equal to four such that the product width and height is less than or equal to the display width and height, respectively
- Real: no scaling
- Fitted: scale the framebuffer up to four times larger or eight times smaller to fit exactly within the display while preserving aspect ratio

In integer and real modes, a few lines may be cropped from the top and bottom to preserve aspect ratio of the framebuffer.

#### PS1 aspect ratio modes

PS1 aspect ratio modes only take effect in integer, real, and fitted scaling modes and is applied before scaling.

- Pixel: aspect ratio of the framebuffer
- 4∶3: aspect ratio is forced to 4∶3
- 16∶9: aspect ratio is forced to 16∶9

Scaling and PS1 aspect ratio will not apply when

- in integer and real modes, if the framebuffer is already larger than the display
- in fitted mode, if the framebuffer is too big to be scaled
- in 4∶3 and 16∶9 modes, aspect ratio is too different to be forced

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

Use the provided configuration app to change settings instantly without needing to close the foreground application or needing to reboot.

## Scaling test

The scaling test program shows horizontal and vertical lines 1 pixel wide alternating between black and white. Use the left and right buttons to cycle between framebuffer resolutions.

**Warning:** Do not use when HDMI is set to interlaced mode.

## Building

Use [DolceSDK](https://forum.devchroma.nl/index.php/topic,129.0.html) and [libvita2d_sys](https://github.com/GrapheneCt/libvita2d_sys) to build. The following modifications to libvita2d_sys are required to reduce memory usage:

```
DEFAULT_TEMP_POOL_SIZE      128 KiB
PARAMETER_BUFFER_SIZE       256 KiB
VDM_RING_BUFFER_SIZE         32 KiB
VERTEX_RING_BUFFER_SIZE     128 KiB
FRAGMENT_RING_BUFFER_SIZE    64 KiB

Remove the stencil buffer
```

## Credits

- Bounty backers: [ScHlAuChii, eleriaqueen, mansjg, TG](https://www.bountysource.com/issues/78540965-native-resolution-output-for-pstv)
- Video comparisons: Zodasaur
- SceLowio: xerpi
- Author: 浅倉麗子

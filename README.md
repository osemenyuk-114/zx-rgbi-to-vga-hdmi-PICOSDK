# zx-rgbi-to-vga-hdmi-PICO-SDK

A converter for ZX Spectrum RGBI video signals to modern VGA and HDMI displays.  
This repository contains code from [zx-rgbi-to-vga-hdmi](https://github.com/osemenyuk-114/zx-rgbi-to-vga-hdmi), modified to use the native Pico SDK and tools instead of the Arduino framework.

For detailed hardware and original software information, see the upstream projects:  

🔗 [RGBI_TO_VGA](https://github.com/tchv71/RGBI_TO_VGA)
🔗 [ZX_RGBI2VGA-HDMI](https://github.com/AlexEkb4ever/ZX_RGBI2VGA-HDMI/)

## Documentation

- [OSD Menu Guide](docs/OSD_MENU_GUIDE.md) - local button controls, menu tree, and tuning workflow.
- [FF OSD Guide](docs/FF_OSD_GUIDE.md) - Gotek/FlashFloppy I2C wiring, protocol modes, and host configuration.
- [VGA Timings](docs/VGA_TIMINGS.md) - supported VGA/DVI timing tables.

---

## Features

### Software

- **Video Output:**
  - VGA output with selectable resolutions: 640×480 @60Hz, 800×600 @60Hz, 1024×768 @60Hz, 1280×1024 @60Hz.
  - HDMI (DVI) resolutions: 640×480 @60Hz and 720×576 @50Hz.
  - Optional scanline effect on the VGA output at higher resolutions for a retro look.
  - "NO SIGNAL" message when no input is detected.
- **On-Screen Display (OSD) Menu:**
  - Full-featured graphical menu system overlaid on video output.
  - Three-button control (UP, DOWN, SEL) with live tuning and save-to-flash support.
  - Quick VGA/DVI toggle via long SEL press (5 seconds).
  - Auto-timeout after 10 seconds of inactivity.
  - See [OSD Menu Guide](docs/OSD_MENU_GUIDE.md) for detailed usage instructions.
- **FlashFloppy / Gotek OSD Support:**
  - Can act as an external I2C OSD for a Gotek running FlashFloppy.
  - Supports both native FF protocol and HD44780-compatible LCD emulation.
  - Runtime enable/disable and protocol switching are available from OSD and serial menus.
  - See [FF OSD Guide](docs/FF_OSD_GUIDE.md) for wiring and configuration details.
- **Configuration via Serial Terminal:**
  - Alternative text-based menu system for headless configuration.
  - Frequency presets for self-synchronizing capture mode (supports ZX Spectrum 48K/128K pixel clocks).
  - Real-time adjustment of all parameters (changes applied immediately).
  - Settings can be saved to flash memory without restart.
- **Test/Welcome Screen:** Styled after the ZX Spectrum 128K.

### Hardware

- **Analog to Digital Conversion:** Converts analog RGB to digital RGBI.
  - Based on the project:  
🔗 [RGBtoHDMI](https://github.com/hoglet67/RGBtoHDMI)

---

## Removed Features

- Z80 CLK external clock source. Self-sync capture mode is now preferred.

---

## Recent Improvements

### Performance Improvements

- **Video Output Optimization**: Streamlined DMA handling for both VGA and DVI/HDMI output modes, resulting in more efficient memory usage and cleaner code structure.
- **Buffer Management**: Simplified buffer switching mechanisms for improved video processing performance.

### Code Quality

- **Settings Integrity**: CRC-32 validation on saved settings — corrupted or uninitialized flash data is detected on boot and automatically replaced with safe defaults.
- **FF OSD Integration**: Added dedicated FlashFloppy/Gotek I2C OSD support, including protocol switching and separate documentation for setup and usage.
- **FF OSD Runtime Control**: FF OSD can be enabled/disabled and the protocol switched at runtime; both operations trigger a full I2C re-initialization on the next Core 1 loop cycle.
- **Memory Optimization**: Reduced unnecessary memory allocations and pointer complexity in video output modules.
- **Architecture Refinements**: Better separation of concerns between video input capture and output generation systems.
- **Maintainability**: Cleaner code structure while preserving critical hardware-specific requirements for reliable video processing.

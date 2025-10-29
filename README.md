# zx-rgbi-to-vga-hdmi-PICO-SDK

A converter for ZX Spectrum RGBI video signals to modern VGA and HDMI displays.  
This repository contains code from [zx-rgbi-to-vga-hdmi](https://github.com/osemenyuk-114/zx-rgbi-to-vga-hdmi), modified to use the native PICO SDK and tools instead of Arduino framework.

For detailed hardware and original software information, see the upstream projects:  

🔗 [RGBI_TO_VGA](https://github.com/tchv71/RGBI_TO_VGA)
🔗 [ZX_RGBI2VGA-HDMI](https://github.com/AlexEkb4ever/ZX_RGBI2VGA-HDMI/)

---

## Features

### Software

- **Video Output:**
  - VGA output with selectable resolutions: 640×480 @60Hz, 800×600 @60Hz, 1024×768 @60Hz, 1280×1024 @60Hz.
  - HDMI (DVI) resolutions: 640×480 @60Hz and 720×576 @50Hz.
  - Optional scanline effect on the VGA output at higher resolutions for a retro look.
  - "NO SIGNAL" message when no input is detected.
- **Configuration via Serial Terminal:**
  - Text-based menu system.
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

- **Memory Optimization**: Reduced unnecessary memory allocations and pointer complexity in video output modules.
- **Architecture Refinements**: Better separation of concerns between video input capture and output generation systems.
- **Maintainability**: Cleaner code structure while preserving critical hardware-specific requirements for reliable video processing.

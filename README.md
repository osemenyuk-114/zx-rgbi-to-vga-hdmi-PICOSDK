# zx-rgbi-to-vga-hdmi-PICO-SDK

A converter for ZX Spectrum RGBI video signals to modern VGA and HDMI displays.  
This repository contains code from [zx-rgbi-to-vga-hdmi](https://github.com/osemenyuk-114/zx-rgbi-to-vga-hdmi), modified to use the native PICO SDK and tools instead of Arduino framework.

For detailed hardware and original software information, see the upstream projects:  

- [RGBI_TO_VGA](https://github.com/tchv71/RGBI_TO_VGA)
- [ZX_RGBI2VGA-HDMI](https://github.com/AlexEkb4ever/ZX_RGBI2VGA-HDMI/)

---

## Features

### Software

- **Video Output:**
  - VGA output with selectable resolutions: 640x480, 800x600, 1024x768, and 1280x1024.
  - HDMI (DVI) output support at a fixed resolution of 640x480.
  - Optional scanline effect at higher resolutions for a retro look.
  - "NO SIGNAL" message when no input is detected.
- **Configuration via Serial Terminal:**
  - Text-based menu system.
  - Frequency presets for self-synchronizing capture mode (supports ZX Spectrum 48K/128K pixel clocks).
  - Real-time adjustment of all parameters (changes are applied immediately).
  - Settings can be saved to flash memory without a restart.
- **Test/Welcome Screen:** Styled after the ZX Spectrum 128K.

---

## Removed Features

- Z80 CLK external clock source. Self-sync capture mode is now preferred.

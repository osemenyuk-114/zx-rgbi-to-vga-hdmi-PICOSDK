# CLAUDE.md — Project Context for Claude Code

## Project
ZX Spectrum RGBI-to-VGA/HDMI converter. Raspberry Pi Pico (RP2040), Pico SDK 2.2.0, C11.

## Build

```bash
# Configure (only needed once or after CMakeLists.txt changes)
cmake -B build -G Ninja

# Compile
ninja -C build

# Upload to Pico (device must be connected)
picotool load build/ZX_RGBI_TO_VGA_HDMI.elf -fx
```

VS Code tasks are also available: "Compile Project", "Upload with Auto-Reset", "Reset to BOOTSEL".

The build uses CMake + Ninja. Toolchain is arm-none-eabi-gcc via Pico SDK. Optimization is `-O3`.

## Architecture

### Dual-Core Design
- **Core 0**: Video output (VGA/DVI DMA ISR), OSD update loop, serial menu. Entry: `setup()` → `loop()` in `main.c`.
- **Core 1**: RGB capture (DMA ISR), LED heartbeat, FlashFloppy I2C OSD. Entry: `setup1()` → `loop1()` in `main.c`. Runs from RAM (`__not_in_flash_func`).

### PIO (Programmable I/O)
- `pio1 SM0`: RGB capture — `pio_capture_0` (self-clocked) or `pio_capture_1` (external clock). Defined in `programs.pio`.
- `pio0 SM0`: Video output — `pio_vga` (8-bit parallel) or `pio_dvi` (6-bit data + 2-bit side-set clock).

### DMA
6 channels total, all using ping-pong (data + control channel pairs):
- 2 for capture (IRQ1)
- 2 for VGA output (IRQ0)
- 2 for DVI output (IRQ0)

### Video Pipeline
1. **Capture** (`rgb_capture.c`): PIO samples 7 pins (B,G,R,I,HS,VS,F) → DMA ping-pong 2×8KB buffers → ISR unpacks 32-bit words MSB→LSB, packs 4-bit pixel pairs, detects H/V sync.
2. **Buffer** (`v_buf.c`): Triple-buffer, 3×63KB. 416×304 pixels, 4-bit packed (2 pixels/byte). Producer/consumer model with show-flags.
3. **Output** (`vga.c` / `dvi.c`): Palette lookup, scanline rendering with OSD compositing. VGA uses 8-bit parallel output. DVI uses software TMDS encoding with pre-computed 64-bit palette entries.

## Source File Map

| File | Role |
|---|---|
| `src/main.c` | Entry point, core 0/1 setup and main loops |
| `src/g_config.h` | Master config: board pins, enums, structs, constants |
| `src/g_config.c` | Video mode timing tables, `g_v_buf[]` allocation |
| `src/rgb_capture.c/h` | PIO capture, DMA ping-pong, sync detection |
| `src/v_buf.c/h` | Triple-buffer management |
| `src/video_output.c/h` | Output abstraction, VGA/DVI auto-detect, welcome screen |
| `src/vga.c/h` | VGA output: PIO, palette, DMA scanline ISR |
| `src/dvi.c/h` | DVI/HDMI: TMDS encoding, DMA ISR |
| `src/osd.c/h` | Common OSD: 240×120 pixel buffer, font, buttons |
| `src/osd_menu.c/h` | Interactive settings menu (conditional: `OSD_MENU_ENABLE`) |
| `src/ff_osd.c/h` | FlashFloppy I2C OSD (conditional: `OSD_FF_ENABLE`) |
| `src/settings.c/h` | Flash persistence, CRC32, load/save (halts Core 1) |
| `src/serial_menu.c/h` | USB serial configuration interface |
| `src/font.h` | 8×8 bitmap font data |
| `src/pio_programs.h` | PIO header include, runtime instruction patching opcodes |
| `src/programs.pio` | PIO assembly for all 4 programs |

## Key Constants

- `V_BUF_W` = 416, `V_BUF_H` = 304, `V_BUF_SZ` = 63232
- `CAP_DMA_BUF_SIZE` = 8192 (1024 × 8 bytes per DMA transfer)
- OSD: 240×120 px, 30×15 text grid, 8×8 font
- Capture frequency: 6–8 MHz (default 7), delay: 0–31 PIO cycles
- System clocks: 240–270 MHz depending on video mode

## Board Variants (defined in `g_config.h`)

Select by uncommenting exactly one `#define BOARD_*`:
- `BOARD_36LJU22` (default): DVI/VGA pin 8, capture pin 0, auto-detect, I2C 20/21
- `BOARD_11XGA24`: DVI pin 0, VGA pin 8, capture pin 16, no FF OSD
- `BOARD_LEO_REV3`: PS/2 keyboard, CH446Q crossbar, different button pins
- `BOARD_09LJV23`: DVI/VGA pin 7, capture pin 0

## Conditional Compilation

CMake flags in `CMakeLists.txt` (lines 42–43):
```cmake
set(OSD_MENU_ENABLE 1)   # OSD settings menu
set(OSD_FF_ENABLE 1)     # FlashFloppy I2C OSD
```
These control source file inclusion, library linking, and preprocessor defines. Either/both/neither can be enabled.

When `OSD_FF_ENABLE` is set, `pico_i2c_slave` library is fetched from GitHub via FetchContent.

## Critical Implementation Notes

- **Flash writes** (`save_settings()`): Must halt Core 1 and disable all interrupts. Flash is XIP — code executing from flash will crash during erase/program.
- **PIO capture byte order**: `sm_config_set_in_shift(shift_right=false)` with 32-bit pushes means oldest byte is in bits [31:24]. DMA handler must unpack MSB→LSB.
- **DMA ping-pong timing**: Control-channel source pointer sequencing in `dma_handler_capture()` is timing-sensitive. Changing ping-pong index update order can cause noisy capture.
- **DVI mode limit**: TMDS bandwidth limits DVI to modes ≤ 720×576@50Hz (`VIDEO_MODE_DVI_MAX`).
- **OSD compositing** happens inline during output scanline ISRs (both VGA and DVI paths).

## Video Modes (7 total)

| Mode | Resolution | Sys Clock | DVI? |
|---|---|---|---|
| 0 | 640×480 @60Hz | 252 MHz | Yes |
| 1 | 720×576 @50Hz | 270 MHz | Yes |
| 2 | 800×600 @60Hz | 240 MHz | VGA only |
| 3 | 1024×768 @60Hz div3 | 260 MHz | VGA only |
| 4 | 1024×768 @60Hz div4 | 260 MHz | VGA only |
| 5 | 1280×1024 @60Hz div3 | 252 MHz | VGA only |
| 6 | 1280×1024 @60Hz div4 | 243 MHz | VGA only |

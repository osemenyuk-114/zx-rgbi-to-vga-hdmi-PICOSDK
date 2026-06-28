# Keyboard Guide

PS/2 and USB keyboards are supported for ZX Spectrum key emulation and OSD/Gotek control.

## Supported Connections

| Interface | Connector     | Notes                        |
|-----------|---------------|------------------------------|
| PS/2      | Mini-DIN-6    | PIO-based driver, IRQ-driven |
| USB       | Micro-USB OTG | TinyUSB Host, boot protocol  |

Both PS/2 and USB can work simultaneously — key states are OR-merged.

---

## OSD Menu Control

| Key        | Action                                            |
|------------|---------------------------------------------------|
| **F9**     | Open / toggle OSD setup menu                      |
| **↑ / ←**  | Navigate up / adjust value up                     |
| **↓ / →**  | Navigate down / adjust value down                 |
| **Enter**  | Select item / enter tuning mode                   |
| **Esc**    | Exit tuning → back to submenu → main menu → close |

Arrow keys have controlled repeat: 400ms initial delay, then 80ms rate.
Hold duration enables progressive acceleration for frequency adjustment.

---

## Gotek / FlashFloppy Control

| Key       | Action                              |
|-----------|-------------------------------------|
| **F10**   | Toggle Gotek keyboard mode (on/off) |
| **← / ↑** | Gotek LEFT (previous file)          |
| **→ / ↓** | Gotek RIGHT (next file)             |
| **Enter** | Gotek SELECT (load file)            |

When Gotek mode is active:

- Arrow keys and Enter are routed to Gotek via I2C (not to ZX Spectrum).
- Gotek OSD is displayed on screen regardless of Gotek backlight state.
- Press **F10** again to deactivate and release the keyboard.

---

## NMI / RESET

Behavior depends on the output hardware variant:

| Key     | CH446Q                         | EPM3256                               |
|---------|--------------------------------|---------------------------------------|
| **F11** | Closes switch Y5:X10 (NMI)     | Sends NMI bit in SPI frame (bit 0)    |
| **F12** | Closes switch Y6:X11 (RESET)   | Sends RESET bit in SPI frame (bit 1)  |

In all cases the signal is **level-based**: it follows the key state (active while held, released when key is released).

The EPM3256 CPLD receives the NMI/RESET bits on every SPI frame and emulates the corresponding hardware button presses on the ZX Spectrum board.

---

## USB Mouse (Kempston)

USB mice are supported in SPI/EPM3256 firmware builds.

| Button        | Default mapping (D-bit) |
|---------------|-------------------------|
| Right button  | D0                      |
| Left button   | D1                      |

The default matches the original hardware schematic (right→D0, left→D1).  
**F6** toggles the mapping: swapped = left→D0, right→D1.

The mouse X/Y position is accumulated (0–255, wrapping) in Kempston format.  
Y axis is inverted (USB reports screen-down as positive; Kempston expects the opposite).

---

## ZX Spectrum Key Mapping

### Direct Mappings

Letters A–Z, numbers 0–9, Enter, Space map directly to ZX equivalents.

### Modifier Keys

| Key                | ZX Equivalent |
|--------------------|---------------|
| Left / Right Shift | CAPS SHIFT    |
| Left / Right Ctrl  | SYMBOL SHIFT  |

### CAPS SHIFT Combinations

| Key       | ZX Combination | Function      |
|-----------|----------------|---------------|
| Backspace | CS + 0         | DELETE        |
| Delete    | CS + 0         | DELETE        |
| Tab       | CS + 1         | EDIT          |
| Caps Lock | CS + 2         | CAPS LOCK     |
| Page Up   | CS + 3         | TRUE VIDEO    |
| Page Down | CS + 4         | INVERSE VIDEO |
| Escape    | CS + SPACE     | BREAK         |
| ↑         | CS + 7         | Cursor up     |
| ↓         | CS + 6         | Cursor down   |
| ←         | CS + 5         | Cursor left   |
| →         | CS + 8         | Cursor right  |

### SYMBOL SHIFT Combinations

| Key           | ZX Combination | Output |
|---------------|----------------|--------|
| ' (Quote)     | SS + P         | "      |
| , (Comma)     | SS + N         | ,      |
| . (Period)    | SS + M         | .      |
| / (Slash)     | SS + V         | /      |
| ; (Semicolon) | SS + O         | ;      |
| - (Minus)     | SS + J         | -      |
| = (Equals)    | SS + L         | =      |

### Numeric Keypad

| Key       | ZX Equivalent |
|-----------|---------------|
| Num 0–9   | 0–9           |
| Num Enter | ENTER         |
| Num /     | SS + V (/)    |
| Num *     | SS + B (*)    |
| Num -     | SS + J (-)    |
| Num +     | SS + K (+)    |
| Num .     | SS + M (.)    |

---

## ZX Spectrum Keyboard Matrix Reference

The ZX Spectrum uses an 8×5 key matrix read via address lines A8–A15 and data lines D0–D4.
Each half-row is selected by driving the corresponding address line low.

```text
         ┌─────────────────────────────────────────────────────┐
         │     ┌─────────────────────────────────────────┐     │
         │     │     ┌─────────────────────────────┐     │     │
         │     │     │     ┌─────────────────┐     │     │     │
         │     │     │     │     ┌─────┐     │     │     │     │
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
 A11/ ─┤ 1 ├─┤ 2 ├─┤ 3 ├─┤ 4 ├─┤ 5 │ │ 6 ├─┤ 7 ├─┤ 8 ├─┤ 9 ├─┤ 0 ├─ A12/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
 A10/ ─┤ Q ├─┤ W ├─┤ E ├─┤ R ├─┤ T │ │ Y ├─┤ U ├─┤ I ├─┤ O ├─┤ P ├─ A13/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
  A9/ ─┤ A ├─┤ S ├─┤ D ├─┤ F ├─┤ G │ │ H ├─┤ J ├─┤ K ├─┤ L ├─┤Ent├─ A14/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
  A8/ ─┤ CS├─┤ Z ├─┤ X ├─┤ C ├─┤ V │ │ B ├─┤ N ├─┤ M ├─┤ SS├─┤ SP├─ A15/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
         D0    D1    D2    D3    D4    D4    D3    D2    D1    D0
```

CS = CAPS SHIFT, SS = SYMBOL SHIFT, SP = SPACE

### Extended Key Functions

CAPS SHIFT combinations:

| Keys     | Function    | Keys     | Function  |
|----------|-------------|----------|-----------|
| CS + 1   | EDIT        | CS + 6   | DOWN      |
| CS + 2   | CAPS LOCK   | CS + 7   | UP        |
| CS + 3   | TRUE VIDEO  | CS + 8   | RIGHT     |
| CS + 4   | INV VIDEO   | CS + 9   | GRAPH     |
| CS + 5   | LEFT        | CS + 0   | DELETE    |
| CS + SP  | BREAK       | CS + SS  | EXT MODE  |

SYMBOL SHIFT combinations (punctuation):

| Keys   | Output | Keys   | Output |
|--------|--------|--------|--------|
| SS + N | ,      | SS + O | ;      |
| SS + M | .      | SS + P | "      |

---

## Implementation Notes

- Mapping table: `zx_kbd.c` (`zx_key_map[]` with fast LUT for O(1) lookup).
- ZX output: CH446Q analog switch IC, delta-apply (only changed switches sent).
- OSD bridge: `osd_kbd.c` — Core 1 intercepts hotkeys, Core 0 injects virtual buttons.
- USB host: `usb_kbd.c` — `hid_to_universal[]` lookup table, boot protocol reports.
- PS/2 driver: `ps2_kbd.c` — PIO state machine, IRQ on RX FIFO, scancode Set 2.

### Build Configuration

Keyboard support is enabled per-board in `g_config.h`:

| Flag             | Effect                             |
|------------------|------------------------------------|
| `PS2_KBD_ENABLE` | Enable PS/2 keyboard (PIO driver)  |
| `USB_KBD_ENABLE` | Enable USB keyboard (TinyUSB Host) |
| `KBD_ENABLE`     | Auto-derived: PS2 or USB enabled   |
| `SPI_KB_ENABLE`  | EPM3256 SPI output (enables mouse) |
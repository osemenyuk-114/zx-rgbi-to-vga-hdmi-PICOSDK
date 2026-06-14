/**
 * kbd.h - Universal keyboard dispatcher
 *
 * Polls input backends (PS/2, USB), merges state, tracks edges,
 * and routes to ZX Spectrum output backend (CH446Q).
 */

#pragma once

void kbd_init(void);

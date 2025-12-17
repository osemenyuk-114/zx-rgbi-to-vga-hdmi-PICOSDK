#pragma once

#define FF_OSD_BUTTON_LEFT 1
#define FF_OSD_BUTTON_RIGHT 2
#define FF_OSD_BUTTON_SELECT 4

#define FF_OSD_H_OFFSET_MIN 1
#define FF_OSD_H_OFFSET_MAX 199
#define FF_OSD_V_OFFSET_MIN 2
#define FF_OSD_V_OFFSET_MAX 299

// Structure definitions
typedef struct ff_osd_config_t
{
    uint16_t h_offset;
    uint16_t v_offset;
    uint16_t min_cols;
    uint16_t max_cols;
    uint16_t rows;
} ff_osd_config_t;

typedef struct ff_osd_display_t
{
    char text[4][40];
    uint8_t rows;
    uint8_t cols;
    uint8_t heights;
    bool on;
} ff_osd_display_t;

typedef struct ff_osd_info_t
{
    uint8_t protocol_ver;
    uint8_t fw_major, fw_minor;
    uint8_t buttons;
} ff_osd_info_t;

extern ff_osd_config_t ff_osd_config;
extern ff_osd_display_t ff_osd_display;
extern uint8_t ff_osd_buttons_rx;

void ff_osd_update();
void ff_osd_i2c_process();
void ff_osd_i2c_init();
void set_osd_buttons(uint8_t buttons);
void ff_osd_config_process(uint8_t b);
uint16_t set_ff_osd_h_offset(int16_t h_offset);
uint16_t set_ff_osd_v_offset(int16_t v_offset);
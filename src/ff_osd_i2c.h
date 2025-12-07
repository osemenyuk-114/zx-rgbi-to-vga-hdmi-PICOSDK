#pragma once

/* Structure definitions */
typedef struct display_t
{
    char text[4][40];
    uint8_t rows;
    uint8_t cols;
    uint8_t heights;
    bool on;
} display_t;

typedef struct i2c_osd_info_t
{
    uint8_t protocol_ver;
    uint8_t fw_major, fw_minor;
    uint8_t buttons;
} i2c_osd_info_t;

typedef struct config_t
{
    uint16_t min_cols, max_cols;
    uint16_t rows;
} config_t;

extern display_t i2c_display;

void i2c_process();
void setup_i2c_slave();

#define OSD_BUTTON_LEFT 1
#define OSD_BUTTON_RIGHT 2
#define OSD_BUTTON_SELECT 4

void set_osd_buttons(uint8_t buttons);
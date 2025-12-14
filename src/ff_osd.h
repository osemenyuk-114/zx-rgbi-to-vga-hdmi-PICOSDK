#pragma once

typedef struct ff_osd_config_t 
{
    uint16_t h_off;
    uint16_t v_off;
    uint16_t min_cols;
    uint16_t max_cols;
    uint16_t rows;
} ff_osd_config_t;

extern ff_osd_config_t ff_osd_config;

void ff_osd_update();

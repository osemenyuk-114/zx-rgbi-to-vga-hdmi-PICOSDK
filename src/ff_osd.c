#include <stdio.h>
#include <string.h>

#include "g_config.h"
#include "osd.h"
#include "ff_osd.h"
#include "ff_osd_i2c.h"
#include "video_output.h"

const static ff_osd_config_t default_ff_osd_config = {
    .h_off = 42,
    .v_off = 50,
    .min_cols = 16,
    .max_cols = 40,
    .rows = 2,
};

ff_osd_config_t ff_osd_config;

void ff_osd_update()
{
    if (!osd_state.enabled)
        return;

    // FlashFloppy OSD takes priority if active
    if (i2c_display.on)
    {
        osd_mode.columns = i2c_display.cols;
        osd_mode.rows = 4;
        osd_mode.width = osd_mode.columns * OSD_FONT_WIDTH;
        osd_mode.height = osd_mode.rows * OSD_FONT_HEIGHT;
        osd_mode.buffer_size = osd_mode.width * osd_mode.height / 2;

        set_osd_position(1);

        /*
            // Show debug info about the I2C display state
            char debug[40];
            snprintf(debug, sizeof(debug), "I2C: on=%d r=%d c=%d h=%02x",
                     i2c_display.on, i2c_display.rows, i2c_display.cols, i2c_display.heights);
            osd_text_print(0, 0, debug, OSD_COLOR_SELECTED, OSD_COLOR_BACKGROUND, 0);
        // osd_text_printf(1, 0, OSD_COLOR_SELECTED, OSD_COLOR_BACKGROUND, 0, "I2C Buttons: %02x", i2c_buttons_rx);
        */

        uint8_t start_row = 0;

        // Render each row from i2c_display
        for (uint8_t row = 0; (row < i2c_display.rows) && (row < 4); row++)
        {
            uint8_t osd_row = start_row + row;

            if (osd_row >= osd_mode.rows)
                break;

            // Check if this row should be double-height
            uint8_t is_double_height = (i2c_display.heights >> row) & 1;

            osd_text_heights[osd_row] = is_double_height;

            // Calculate starting column to center the text horizontally
            uint8_t text_len = strnlen(i2c_display.text[row], i2c_display.cols);

            if (text_len == 0)
                continue; // Skip empty rows

            // Use bright colors for double-height text, normal colors otherwise
            uint8_t fg_color = is_double_height ? OSD_COLOR_TEXT : OSD_COLOR_DIMMED;

            // Copy the text row and clean non-printable characters
            char row_text[41];

            uint8_t out_pos = 0;

            for (uint8_t col = 0; col < text_len && col < i2c_display.cols && col < osd_mode.columns; col++)
            {
                char c = i2c_display.text[row][col];
                // Skip non-printable characters
                if (c < 32 || c > 126)
                    c = ' ';
                row_text[out_pos++] = c;
            }
            row_text[out_pos] = '\0';

            // Print the entire row at once with the appropriate height
            osd_text_print(osd_row, 0, row_text, fg_color, OSD_COLOR_BACKGROUND, is_double_height);
        }

        // Always mark that text buffer needs to be rendered
        osd_state.text_updated = true;
        osd_state.needs_redraw = true;
    }

    osd_render_text_to_buffer();

    osd_state.visible = i2c_display.on;
}

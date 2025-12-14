#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "hardware/timer.h"

#include "g_config.h"
#include "osd.h"
#include "font.h"

#ifdef OSD_MENU_ENABLE
#include "osd_menu.h"
#endif

#ifdef OSD_FF_ENABLE
#include "ff_osd.h"
#endif

// Global OSD state and buffers
osd_state_t osd_state = {
    .enabled = false,
    .visible = false,
    .needs_redraw = true,
    .text_updated = true};

osd_mode_t osd_mode = {
    .start_x = 0,
    .end_x = OSD_WIDTH,
    .start_y = 0,
    .end_y = OSD_HEIGHT,
    .width = OSD_WIDTH,
    .height = OSD_HEIGHT,
    .buffer_size = OSD_BUFFER_SIZE,
    .columns = OSD_COLUMNS,
    .rows = OSD_ROWS,
    .text_buffer_size = OSD_TEXT_BUFFER_SIZE};

uint8_t osd_buffer[OSD_BUFFER_SIZE];
char osd_text_buffer[OSD_TEXT_BUFFER_SIZE];
uint8_t osd_text_colors[OSD_TEXT_BUFFER_SIZE];
uint8_t osd_text_heights[OSD_ROWS];

void osd_init()
{ // Initialize OSD state
    memset(&osd_state, 0, sizeof(osd_state));

    osd_state.enabled = true;
    osd_state.needs_redraw = true;
    osd_state.text_updated = true;
    // Initialize text buffer
    osd_clear_text_buffer();
    // Clear overlay buffer
    osd_clear_buffer();

#ifdef OSD_MENU_ENABLE
    // Initialize menu-specific components
    osd_menu_init();
#endif
}

void osd_show()
{
    if (!osd_state.enabled)
        return;

    uint64_t current_time = time_us_64();
    osd_state.visible = true;
    osd_state.needs_redraw = true;
    osd_state.show_time = current_time;
    osd_state.last_activity_time = current_time;
    // Draw border once when OSD is activated
    osd_draw_border();
}

void osd_hide()
{
    osd_state.visible = false;
}

void osd_update_activity()
{
    osd_state.last_activity_time = time_us_64();
}

void osd_clear_buffer()
{ // Fill with background color (2 pixels per byte)
    uint8_t bg_color_pair = OSD_COLOR_BACKGROUND | (OSD_COLOR_BACKGROUND << 4);
    memset(osd_buffer, bg_color_pair, OSD_BUFFER_SIZE);
}

void osd_clear_text_buffer()
{ // Clear text buffer but preserve border positions
    uint8_t default_color = (OSD_COLOR_TEXT << 4) | OSD_COLOR_BACKGROUND;

    for (uint8_t row = 0; row < osd_mode.rows; row++)
    {
        for (uint8_t col = 0; col < osd_mode.columns; col++)
        {
            uint16_t pos = row * osd_mode.columns + col;
            // Skip border positions (first and last row, first and last column)
            if (row == 0 || row == osd_mode.rows - 1 || col == 0 || col == osd_mode.columns - 1)
                continue;

            osd_text_buffer[pos] = ' ';
            osd_text_colors[pos] = default_color;
        }

        osd_text_heights[row] = 0;
    }
}

void osd_text_set_char(uint8_t row, uint8_t col, char c, uint8_t fg_color, uint8_t bg_color)
{
    if (row >= osd_mode.rows || col >= osd_mode.columns)
        return;

    uint16_t pos = row * osd_mode.columns + col;
    osd_text_buffer[pos] = c;
    osd_text_colors[pos] = (fg_color << 4) | bg_color;
}

void osd_text_print(uint8_t row, uint8_t col, const char *str, uint8_t fg_color, uint8_t bg_color, uint8_t height)
{
    if (row >= osd_mode.rows)
        return;
    // Set height for this row
    osd_text_heights[row] = height;

    uint16_t row_start = row * osd_mode.columns;
    uint16_t pos = row_start + col;
    uint8_t max_len = osd_mode.columns - col;
    uint8_t packed_color = (fg_color << 4) | bg_color;
    // Fill left padding with spaces (avoid column 0 - left border)
    for (uint8_t i = 1; i < col; i++)
    {
        osd_text_buffer[row_start + i] = ' ';
        osd_text_colors[row_start + i] = packed_color;
    }

    uint8_t i;
    // Copy string characters (avoid last column - right border)
    uint8_t effective_max_len = (col + max_len >= osd_mode.columns) ? (osd_mode.columns - col - 1) : max_len;
    for (i = 0; i < effective_max_len && str[i] != '\0'; i++)
    {
        osd_text_buffer[pos + i] = str[i];
        osd_text_colors[pos + i] = packed_color;
    }
    // Pad with spaces to fill the rest of the row (avoid last column - right border)
    for (; i < effective_max_len; i++)
    {
        osd_text_buffer[pos + i] = ' ';
        osd_text_colors[pos + i] = packed_color;
    }
}

void osd_text_print_centered(uint8_t row, const char *str, uint8_t fg_color, uint8_t bg_color, uint8_t height)
{
    if (row >= osd_mode.rows)
        return;

    uint8_t len = strlen(str);
    // Account for border columns (exclude first and last column)
    uint8_t available_width = osd_mode.columns - 2;
    if (len > available_width)
        len = available_width;

    uint8_t col = 1 + (available_width - len) / 2; // Start at column 1 (after left border)
    uint8_t packed_color = (fg_color << 4) | bg_color;
    uint16_t pos = row * osd_mode.columns;
    // Fill left padding with spaces (start at column 1 to preserve left border)
    for (uint8_t i = 1; i < col; i++)
    {
        osd_text_buffer[pos + i] = ' ';
        osd_text_colors[pos + i] = packed_color;
    }
    // Print centered text
    osd_text_print(row, col, str, fg_color, bg_color, height);
}

void osd_text_printf(uint8_t row, uint8_t col, uint8_t fg_color, uint8_t bg_color, uint8_t height, const char *format, ...)
{
    if (row >= osd_mode.rows)
        return;

    char temp[osd_mode.columns + 1];
    va_list args;
    va_start(args, format);
    vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);

    osd_text_print(row, col, temp, fg_color, bg_color, height);
}

void osd_draw_border()
{ // Top border
    osd_text_set_char(0, 0, OSD_CHAR_BORDER_TL, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);

    for (uint8_t col = 1; col < osd_mode.columns - 1; col++)
        osd_text_set_char(0, col, OSD_CHAR_BORDER_T, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);

    osd_text_set_char(0, osd_mode.columns - 1, OSD_CHAR_BORDER_TR, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);
    // Bottom border
    osd_text_set_char(osd_mode.rows - 1, 0, OSD_CHAR_BORDER_BL, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);

    for (uint8_t col = 1; col < osd_mode.columns - 1; col++)
        osd_text_set_char(osd_mode.rows - 1, col, OSD_CHAR_BORDER_B, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);

    osd_text_set_char(osd_mode.rows - 1, osd_mode.columns - 1, OSD_CHAR_BORDER_BR, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);
    // Left and right borders
    for (uint8_t row = 1; row < osd_mode.rows - 1; row++)
    {
        osd_text_set_char(row, 0, OSD_CHAR_BORDER_L, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);
        osd_text_set_char(row, osd_mode.columns - 1, OSD_CHAR_BORDER_R, OSD_COLOR_BORDER, OSD_COLOR_BACKGROUND);
    }
}

void osd_render_text_to_buffer()
{                          // Render text buffer to pixel buffer
    uint16_t y_offset = 0; // Accumulated Y offset for double-height rows

    for (uint8_t row = 0; row < osd_mode.rows; row++)
    {
        uint8_t height = osd_text_heights[row];

        for (uint8_t col = 0; col < osd_mode.columns; col++)
        {
            uint16_t pos = row * osd_mode.columns + col;
            char c = osd_text_buffer[pos];
            uint8_t packed_color = osd_text_colors[pos];
            uint8_t fg_color = (packed_color >> 4) & 0x0F;
            uint8_t bg_color = packed_color & 0x0F;

            uint16_t x = col * OSD_FONT_WIDTH;
            uint16_t y = row * OSD_FONT_HEIGHT + y_offset;

            osd_draw_char(osd_buffer, osd_mode.width, x, y, c, fg_color, bg_color, height);
        }
        // Add extra vertical space for double-height rows
        if (height)
            y_offset += OSD_FONT_HEIGHT;
    }
}

void osd_draw_char(uint8_t *buffer, uint16_t buf_width, uint16_t x, uint16_t y,
                   char c, uint8_t fg_color, uint8_t bg_color, uint8_t height)
{
    if (c < 0 || c > 255)
        return;

    const uint8_t *char_data = osd_font[(uint8_t)c];
    uint8_t height_multiplier = height ? 2 : 1;

    for (int row = 0; row < OSD_FONT_HEIGHT; row++)
    {
        uint8_t line = char_data[row];

        for (int pixel_row = 0; pixel_row < height_multiplier; pixel_row++)
        {
            uint16_t py = y + row * height_multiplier + pixel_row;

            for (int col = 0; col < OSD_FONT_WIDTH; col++)
            {
                uint16_t px = x + col;
                // Check bounds
                if (px >= buf_width || py >= osd_mode.height)
                    continue;
                // Calculate buffer position (2 pixels per byte)
                int buffer_offset = py * (buf_width / 2) + (px / 2);

                if (buffer_offset >= osd_mode.buffer_size)
                    continue;
                // Determine pixel color
                uint8_t pixel_color = (line & (0x80 >> col)) ? fg_color : bg_color;
                // Set pixel in buffer (2 pixels per byte)
                if (px & 1)
                { // Odd pixel (upper 4 bits)
                    buffer[buffer_offset] = (buffer[buffer_offset] & 0x0F) | (pixel_color << 4);
                }
                else
                { // Even pixel (lower 4 bits)
                    buffer[buffer_offset] = (buffer[buffer_offset] & 0xF0) | (pixel_color & 0x0F);
                }
            }
        }
    }
}

void osd_update()
{
#ifdef OSD_FF_ENABLE
    ff_osd_update();
#endif

#ifdef OSD_MENU_ENABLE
    // Handle menu OSD if FF OSD is not active
    osd_menu_update();
#endif
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/watchdog.h"
#include "pico/stdio.h"

#include "serial_menu.h"

extern "C"
{
#include "g_config.h"
#include "v_buf.h"
#include "settings.h"
#include "rgb_capture.h"
#include "video_output.h"
}

// External variables that need to be accessed
extern settings_t settings;
extern video_out_type_t active_video_output;
extern volatile bool restart_capture;

void print_byte_hex(uint8_t byte)
{
    printf("%02X", byte);
}

void binary_to_string(uint8_t value, bool mask_1, char *str)
{
    uint8_t binary = value;

    for (int i = 0; i < 8; i++)
    {
        str[i] = binary & 0b10000000 ? (mask_1 ? 'X' : '1') : '0';
        binary <<= 1;
    }

    str[8] = '\0';
}

uint32_t string_to_int(const char *value)
{
    char *end;
    long val = strtol(value, &end, 10);
    return (value == end) ? 0 : (uint32_t)val;
}

// Input helper function to consolidate repetitive input reading logic
char get_menu_input(int timeout_ms)
{
    int c = getchar_timeout_us(timeout_ms * 1000);
    return (c == PICO_ERROR_TIMEOUT) ? 0 : (char)c;
}

void print_main_menu()
{
    printf("\n      * ZX RGB(I) to VGA/HDMI ");
    printf("%s", FW_VERSION);
    printf(" *\n\n");

    printf("  o   set video output type (DVI/VGA)\n");
    printf("  v   set video resolution\n");

    if (settings.video_out_type == VGA)
        printf("  s   set scanlines mode\n");

    printf("  b   set buffering mode\n");
    printf("  c   set capture synchronization source\n");
    printf("  f   set capture frequency\n");
    printf("  d   set external clock divider\n");
    printf("  y   set video sync mode\n");
    printf("  t   set capture delay and image position\n");
    printf("  m   set pin inversion mask\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit configuration mode\n");
    printf("  w   save configuration\n");
    printf("  r   restart\n\n");
}

void print_video_out_menu()
{
    printf("\n      * Video resolution *\n\n");

    printf("  1    640x480 @60Hz (div 2)\n");

    switch (settings.video_out_type)
    {
    case DVI:
        printf("  2    720x576 @50Hz (div 2)\n");
        break;

    case VGA:
        printf("  2    800x600 @60Hz (div 2)\n");
        printf("  3   1024x768 @60Hz (div 3)\n");
        printf("  4  1280x1024 @60Hz (div 3)\n");
        printf("  5  1280x1024 @60Hz (div 4)\n");
        break;

    default:
        break;
    }

    printf("\n");
    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_video_out_type_menu()
{
    printf("\n      * Video output type *\n\n");

    printf("  1   DVI\n");
    printf("  2   VGA\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_scanlines_mode_menu()
{
    printf("\n      * Scanlines mode *\n\n");

    printf("  s   change scanlines mode\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_buffering_mode_menu()
{
    printf("\n      * Buffering mode *\n\n");

    printf("  b   change buffering mode\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_cap_sync_mode_menu()
{
    printf("\n      * Capture synchronization source *\n\n");

    printf("  1   self-synchronizing\n");
    printf("  2   external clock\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_capture_frequency_menu()
{
    printf("\n      * Capture frequency *\n\n");

    printf("  1   7000000 Hz (ZX Spectrum  48K)\n");
    printf("  2   7093800 Hz (ZX Spectrum 128K)\n");
    printf("  3   custom\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_ext_clk_divider_menu()
{
    printf("\n      * External clock divider *\n\n");

    printf("  a   increment divider (+1)\n");
    printf("  z   decrement divider (-1)\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_video_sync_mode_menu()
{
    printf("\n      * Video synchronization mode *\n\n");

    printf("  y   change synchronization mode\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_image_tuning_menu()
{
    printf("\n      * Capture delay and image position *\n\n");

    printf("  a   increment delay (+1)\n");
    printf("  z   decrement delay (-1)\n\n");

    printf("  i   shift image UP\n");
    printf("  k   shift image DOWN\n");
    printf("  j   shift image LEFT\n");
    printf("  l   shift image RIGHT\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_pin_inversion_mask_menu()
{
    printf("\n      * Pin inversion mask *\n\n");

    printf("  m   set pin inversion mask\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_test_menu()
{
    printf("\n      * Tests *\n\n");

    printf("  1   draw welcome image (vertical stripes)\n");
    printf("  2   draw welcome image (horizontal stripes)\n");
    printf("  3   draw \"NO SIGNAL\" screen\n");
    printf("  i   show captured frame count\n\n");

    printf("  p   show configuration\n");
    printf("  h   show help (this menu)\n");
    printf("  q   exit to main menu\n\n");
}

void print_video_out_type()
{
    printf("  Video output type ........... ");

    switch (settings.video_out_type)
    {
    case DVI:
        printf("DVI\n");
        break;

    case VGA:
        printf("VGA\n");
        break;

    default:
        break;
    }
}

void print_video_out_mode()
{
    printf("  Video resolution ............ ");

    switch (settings.video_out_mode)
    {
    case MODE_640x480_60Hz:
        printf("640x480 @60Hz\n");
        break;

    case MODE_720x576_50Hz:
        printf("720x576 @50Hz\n");
        break;

    case MODE_800x600_60Hz:
        printf("800x600 @60Hz\n");
        break;

    case MODE_1024x768_60Hz:
        printf("1024x768 @60Hz\n");
        break;

    case MODE_1280x1024_60Hz_d3:
        printf("1280x1024 @60Hz (div 3)\n");
        break;

    case MODE_1280x1024_60Hz_d4:
        printf("1280x1024 @60Hz (div 4)\n");
        break;

    default:
        break;
    }
}

void print_scanlines_mode()
{
    printf("  Scanlines ................... ");

    if (settings.scanlines_mode)
        printf("enabled\n");
    else
        printf("disabled\n");
}

void print_buffering_mode()
{
    printf("  Buffering mode .............. ");

    if (settings.buffering_mode)
        printf("x3\n");
    else
        printf("x1\n");
}

void print_cap_sync_mode()
{
    printf("  Capture sync source ......... ");

    switch (settings.cap_sync_mode)
    {
    case SELF:
        printf("self-synchronizing\n");
        break;

    case EXT:
        printf("external clock\n");
        break;

    default:
        break;
    }
}

void print_capture_frequency()
{
    printf("  Capture frequency ........... ");
    printf("%d", settings.frequency);
    printf(" Hz\n");
}

void print_ext_clk_divider()
{
    printf("  External clock divider ...... ");
    printf("%d\n", settings.ext_clk_divider);
}

void print_capture_delay()
{
    printf("  Capture delay ............... ");
    printf("%d\n", settings.delay);
}

void print_x_offset()
{
    printf("  X offset .................... ");
    printf("%d\n", settings.shX);
}

void print_y_offset()
{
    printf("  Y offset .................... ");
    printf("%d\n", settings.shY);
}

void print_dividers()
{
    uint16_t div_int;
    uint8_t div_frac;

    video_mode_t video_mode = *(video_modes[settings.video_out_mode]);

    printf("\n  System clock frequency ...... ");
    printf("%.1f", (float)clock_get_hz(clk_sys));
    printf(" Hz\n");

    if (settings.cap_sync_mode == SELF)
    {
        printf("  Capture divider ............. ");

        pio_calculate_clkdiv_from_float((float)clock_get_hz(clk_sys) / (settings.frequency * 12.0), &div_int, &div_frac);

        printf("%.8f", (div_int + (float)div_frac / 256));

        printf(" ( ");
        printf("0x");
        print_byte_hex((uint8_t)(div_int >> 8));
        print_byte_hex((uint8_t)(div_int & 0xff));
        print_byte_hex(div_frac);
        printf(" )\n");
    }

    if (settings.video_out_type == VGA)
    {
        printf("  Video output clock divider .. ");

        pio_calculate_clkdiv_from_float(((float)clock_get_hz(clk_sys) * video_mode.div) / video_mode.pixel_freq, &div_int, &div_frac);

        printf("%.8f", (div_int + (float)div_frac / 256));

        printf(" ( ");
        printf("0x");
        print_byte_hex((uint8_t)(div_int >> 8));
        print_byte_hex((uint8_t)(div_int & 0xff));
        print_byte_hex(div_frac);
        printf(" )\n");
    }

    printf("\n");
}

void print_video_sync_mode()
{
    printf("  Video synchronization mode .. ");
    if (settings.video_sync_mode)
        printf("separate\n");
    else
        printf("composite\n");
}

void print_pin_inversion_mask()
{
    char binary_str[9];
    printf("  Pin inversion mask .......... ");
    binary_to_string(settings.pin_inversion_mask, false, binary_str);
    printf("%s\n", binary_str);
}

void print_settings()
{
    printf("\n");
    print_video_out_type();
    print_video_out_mode();

    if (settings.video_out_type == VGA)
        print_scanlines_mode();

    print_buffering_mode();
    print_cap_sync_mode();
    print_capture_frequency();
    print_ext_clk_divider();
    print_video_sync_mode();
    print_capture_delay();
    print_x_offset();
    print_y_offset();
    print_pin_inversion_mask();
    print_dividers();
    printf("\n");
}

void handle_serial_menu()
{
    char inchar = 0;

    while (1)
    {
        sleep_ms(500);

        int c = getchar_timeout_us(1000);
        if (c != PICO_ERROR_TIMEOUT)
        {
            inchar = 'h';
            break;
        }
    }

    printf(" Entering the configuration mode\n\n");

    while (1)
    {
        if (inchar != 'h')
            inchar = get_menu_input(10); // 10ms timeout

        switch (inchar)
        {
        case 'p':
            print_settings();
            inchar = 0;
            break;

        case 'o':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                uint8_t video_out_type = settings.video_out_type;

                switch (inchar)
                {
                case 'p':
                    print_video_out_type();
                    break;

                case 'h':
                    print_video_out_type_menu();
                    break;

                case '1':
                    settings.video_out_type = DVI;
                    settings.video_out_mode = VIDEO_OUT_MODE_DEF;
                    print_video_out_type();
                    break;

                case '2':
                    settings.video_out_type = VGA;
                    settings.video_out_mode = VIDEO_OUT_MODE_DEF;
                    print_video_out_type();
                    break;

                default:
                    break;
                }

                if (video_out_type != settings.video_out_type && active_video_output != settings.video_out_type)
                    printf("  Note: Save config and restart for changes to take effect.\n");

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'v':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                uint8_t video_out_mode = settings.video_out_mode;

                switch (inchar)
                {
                case 'p':
                    print_video_out_mode();
                    break;

                case 'h':
                    print_video_out_menu();
                    break;

                case '1':
                    settings.video_out_mode = MODE_640x480_60Hz;
                    print_video_out_mode();
                    break;

                case '2':
                    if (settings.video_out_type == DVI)
                        settings.video_out_mode = MODE_720x576_50Hz;
                    else
                        settings.video_out_mode = MODE_800x600_60Hz;

                    print_video_out_mode();
                    break;

                case '3':
                    if (settings.video_out_type == VGA)
                    {
                        settings.video_out_mode = MODE_1024x768_60Hz;
                        print_video_out_mode();
                    }

                    break;

                case '4':
                    if (settings.video_out_type == VGA)
                    {
                        settings.video_out_mode = MODE_1280x1024_60Hz_d3;
                        print_video_out_mode();
                    }

                    break;

                case '5':
                    if (settings.video_out_type == VGA)
                    {
                        settings.video_out_mode = MODE_1280x1024_60Hz_d4;
                        print_video_out_mode();
                    }

                    break;

                default:
                    break;
                }

                if (video_out_mode != settings.video_out_mode && active_video_output == settings.video_out_type)
                {
                    stop_video_output();
                    start_video_output(active_video_output);
                    // capture PIO clock divider needs to be adjusted for new system clock frequency set in start_video_output()
                    set_capture_frequency(settings.frequency);
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 's':
        {
            if (settings.video_out_type != VGA)
            {
                inchar = 0;
                break;
            }

            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                switch (inchar)
                {
                case 'p':
                    print_scanlines_mode();
                    break;

                case 'h':
                    print_scanlines_mode_menu();
                    break;

                case 's':
                    settings.scanlines_mode = !settings.scanlines_mode;
                    print_scanlines_mode();
                    set_scanlines_mode();
                    break;

                default:
                    break;
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'b':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                switch (inchar)
                {
                case 'p':
                    print_buffering_mode();
                    break;

                case 'h':
                    print_buffering_mode_menu();
                    break;

                case 'b':
                    settings.buffering_mode = !settings.buffering_mode;
                    print_buffering_mode();
                    set_buffering_mode(settings.buffering_mode);
                    break;

                default:
                    break;
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'c':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                uint8_t cap_sync_mode = settings.cap_sync_mode;

                switch (inchar)
                {
                case 'p':
                    print_cap_sync_mode();
                    break;

                case 'h':
                    print_cap_sync_mode_menu();
                    break;

                case '1':
                    settings.cap_sync_mode = SELF;
                    print_cap_sync_mode();
                    break;

                case '2':
                    settings.cap_sync_mode = EXT;
                    print_cap_sync_mode();
                    break;

                default:
                    break;
                }

                if (cap_sync_mode != settings.cap_sync_mode)
                    restart_capture = true;

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'f':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                uint32_t frequency = settings.frequency;

                switch (inchar)
                {
                case 'p':
                    print_capture_frequency();
                    break;

                case 'h':
                    print_capture_frequency_menu();
                    break;

                case '1':
                    settings.frequency = 7000000;
                    print_capture_frequency();
                    break;

                case '2':
                    settings.frequency = 7093800;
                    print_capture_frequency();
                    break;

                case '3':
                {
                    char frequency_str[16] = "";
                    int str_len = 0;
                    uint32_t frequency_int = 0;

                    printf("  Enter frequency: ");

                    while (1)
                    {
                        int c = getchar_timeout_us(10000);

                        if (c == PICO_ERROR_TIMEOUT)
                            continue;

                        if (c >= '0' && c <= '9' && str_len < 15)
                        {
                            printf("%c", c);
                            frequency_str[str_len++] = c;
                            frequency_str[str_len] = '\0';
                        }
                        else if (c == 8 || c == 127) // Backspace
                        {
                            if (str_len > 0)
                            {
                                str_len--;
                                frequency_str[str_len] = '\0';
                                printf("\b \b");
                            }
                        }
                        else if (c == '\r' || c == '\n')
                        {
                            printf("\n");
                            if (str_len > 0)
                            {
                                frequency_int = string_to_int(frequency_str);

                                if (frequency_int >= FREQUENCY_MIN && frequency_int <= FREQUENCY_MAX)
                                {
                                    settings.frequency = frequency_int;
                                    print_capture_frequency();
                                    break;
                                }
                            }

                            // Reset for retry
                            str_len = 0;
                            frequency_str[0] = '\0';
                            printf("  Allowed frequency range ..... ");
                            printf("%d", FREQUENCY_MIN);
                            printf(" - ");
                            printf("%d", FREQUENCY_MAX);
                            printf(" Hz\n");
                            printf("  Enter frequency: ");
                        }
                    }

                    break;
                }

                default:
                    break;
                }

                if (frequency != settings.frequency)
                {
                    set_capture_frequency(settings.frequency);
                    // restart video output with new capture frequency value which is used to calculate horizontal margins for some video output modes
                    stop_video_output();
                    start_video_output(active_video_output);
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'd':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                switch (inchar)
                {

                case 'p':
                    print_ext_clk_divider();
                    break;

                case 'h':
                    print_ext_clk_divider_menu();
                    break;

                case 'a':
                    settings.ext_clk_divider = set_ext_clk_divider(settings.ext_clk_divider + 1);
                    print_ext_clk_divider();
                    break;

                case 'z':
                    settings.ext_clk_divider = set_ext_clk_divider(settings.ext_clk_divider - 1);
                    print_ext_clk_divider();
                    break;

                default:
                    break;
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'y':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                switch (inchar)
                {
                case 'p':
                    print_video_sync_mode();
                    break;

                case 'h':
                    print_video_sync_mode_menu();
                    break;

                case 'y':
                    settings.video_sync_mode = !settings.video_sync_mode;
                    print_video_sync_mode();
                    set_video_sync_mode(settings.video_sync_mode);
                    break;

                default:
                    break;
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 't':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                switch (inchar)
                {

                case 'p':
                    print_capture_delay();
                    print_x_offset();
                    print_y_offset();
                    break;

                case 'h':
                    print_image_tuning_menu();
                    break;

                case 'a':
                    settings.delay = set_capture_delay(settings.delay + 1);
                    print_capture_delay();
                    break;

                case 'z':
                    settings.delay = set_capture_delay(settings.delay - 1);
                    print_capture_delay();
                    break;

                case 'i':
                    settings.shY = set_capture_shY(settings.shY + 1);
                    print_y_offset();
                    break;

                case 'k':
                    settings.shY = set_capture_shY(settings.shY - 1);
                    print_y_offset();
                    break;

                case 'j':
                    settings.shX = set_capture_shX(settings.shX + 1);
                    print_x_offset();
                    break;

                case 'l':
                    settings.shX = set_capture_shX(settings.shX - 1);
                    print_x_offset();
                    break;

                default:
                    break;
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'm':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                uint8_t pin_inversion_mask = settings.pin_inversion_mask;

                switch (inchar)
                {
                case 'p':
                    print_pin_inversion_mask();
                    break;

                case 'h':
                    print_pin_inversion_mask_menu();
                    break;

                case 'm':
                {
                    char pin_inversion_mask_str[9] = "";
                    int str_len = 0;

                    printf("  Enter pin inversion mask: ");

                    while (1)
                    {
                        int c = getchar_timeout_us(10000);

                        if (c == PICO_ERROR_TIMEOUT)
                            continue;

                        if ((c == '0' || c == '1') && str_len < 8)
                        {
                            printf("%c", c);
                            pin_inversion_mask_str[str_len++] = c;
                            pin_inversion_mask_str[str_len] = '\0';
                        }
                        else if (c == 8 || c == 127) // Backspace
                        {
                            if (str_len > 0)
                            {
                                str_len--;
                                pin_inversion_mask_str[str_len] = '\0';
                                printf("\b \b");
                            }
                        }
                        else if (c == '\r' || c == '\n')
                        {
                            printf("\n");

                            if (str_len > 0)
                            {
                                uint8_t pin_inversion_mask_int = 0;

                                for (int i = 0; i < str_len; i++)
                                {
                                    pin_inversion_mask_int <<= 1;
                                    pin_inversion_mask_int |= pin_inversion_mask_str[i] == '1' ? 1 : 0;
                                }

                                if (!(pin_inversion_mask_int & ~PIN_INVERSION_MASK))
                                {
                                    settings.pin_inversion_mask = pin_inversion_mask_int;
                                    print_pin_inversion_mask();
                                    break;
                                }
                            }

                            // Reset for retry
                            str_len = 0;
                            pin_inversion_mask_str[0] = '\0';
                            printf("  Allowed inversion mask ...... ");
                            char allowed_mask[9];
                            binary_to_string(PIN_INVERSION_MASK, true, allowed_mask);
                            printf("%s\n", allowed_mask);
                            printf("  Enter pin inversion mask: ");
                        }
                    }

                    break;
                }

                default:
                    break;
                }

                if (pin_inversion_mask != settings.pin_inversion_mask)
                    set_pin_inversion_mask(settings.pin_inversion_mask);

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'T':
        {
            inchar = 'h';

            while (1)
            {
                if (inchar != 'h')
                    inchar = get_menu_input(10);

                switch (inchar)
                {
                case 'p':
                    print_settings();
                    break;

                case 'h':
                    print_test_menu();
                    break;

                case 'i':
                    printf("  Current frame count ......... ");
                    printf("%d\n", frame_count);
                    break;

                case '1':
                case '2':
                case '3':
                {
                    uint32_t frame_count_tmp = frame_count;

                    sleep_ms(100);

                    if (frame_count == frame_count_tmp) // draw the screen only if capture is not active
                    {
                        printf("  Drawing the screen...\n");

                        if (inchar == '1')
                            draw_welcome_screen(*(video_modes[settings.video_out_mode]));
                        else if (inchar == '2')
                            draw_welcome_screen_h(*(video_modes[settings.video_out_mode]));
                        else
                            draw_no_signal(*(video_modes[settings.video_out_mode]));
                    }

                    break;
                }

                default:
                    break;
                }

                if (inchar == 'q')
                {
                    inchar = 'h';
                    break;
                }

                inchar = 0;
            }

            break;
        }

        case 'h':
            print_main_menu();
            inchar = 0;
            break;

        case 'w':
            printf("  Saving settings...\n");
            save_settings(&settings);
            inchar = 0;
            break;

        case 'r':
            printf("  Restarting........\n");
            sleep_ms(100);
            watchdog_reboot(0, 0, 0);
            break;

        default:
            break;
        }

        if (inchar == 'q')
        {
            inchar = 0;
            printf(" Leaving the configuration mode\n\n");
            break;
        }
    }
}

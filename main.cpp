#include "pico.h"
#include "pico/time.h"
#include "hardware/flash.h"
#include "hardware/watchdog.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

#include <stdio.h>
#include <string>
#include <cstring>

#include "gpio.h"
#include "Serial.h"

extern "C"
{
#include "g_config.h"
#include "dvi.h"
#include "rgb_capture.h"
#include "v_buf.h"
#include "vga.h"
}

SerialIo Serial;

settings_t settings;
video_mode_t video_mode;

const int *saved_settings = (const int *)(XIP_BASE + (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE));

volatile bool start_core0 = false;

volatile bool stop_core1 = false;
volatile bool core1_inactive = false;

volatile bool restart_capture = false;
volatile bool capture_active = false;

static void save_settings(settings_t *settings)
{
  Serial.println("  Saving settings...");

  check_settings(settings);

  stop_core1 = true;

  while (!core1_inactive)
    ;

  uint32_t ints = save_and_disable_interrupts();

  flash_range_erase((PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE), FLASH_SECTOR_SIZE);
  flash_range_program((PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE), (uint8_t *)settings, FLASH_PAGE_SIZE);

  restore_interrupts_from_disabled(ints);

  stop_core1 = false;
  core1_inactive = false;
}

void print_byte_hex(uint8_t byte)
{
  if (byte < 16)
    Serial.print("0");

  Serial.print(byte, HEX);
}

String binary_to_string(uint8_t value, bool mask_1)
{
  uint8_t binary = value;
  String str = "";

  for (int i = 0; i < 8; i++)
  {
    str += binary & 0b10000000 ? (mask_1 ? "X" : "1") : "0";
    binary <<= 1;
  }

  return str;
}

uint32_t string_to_int(String value)
{
  char *end;
  long val = std::strtol(value.c_str(), &end, 10);
  return (value.c_str() == end) ? 0 : static_cast<int>(val);
}

void print_main_menu()
{
  Serial.print("\n      * ZX RGB(I) to VGA/HDMI ");
  Serial.print(FW_VERSION);
  Serial.println(" *\n");

  Serial.println("  v   set video output mode");
  Serial.println("  s   set scanlines mode");
  Serial.println("  b   set buffering mode");
  Serial.println("  c   set capture synchronization source");
  Serial.println("  f   set capture frequency");
  Serial.println("  d   set external clock divider");
  Serial.println("  y   set video sync mode");
  Serial.println("  t   set capture delay and image position");
  Serial.println("  m   set pin inversion mask\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit configuration mode");
  Serial.println("  w   save configuration");
  Serial.println("  r   restart\n");
}

void print_video_out_menu()
{
  Serial.println("\n      * Video output mode *\n");

  Serial.println("  1   HDMI   640x480 (div 2)");
  Serial.println("  2   VGA    640x480 (div 2)");
  Serial.println("  3   VGA    800x600 (div 2)");
  Serial.println("  4   VGA   1024x768 (div 3)");
  Serial.println("  5   VGA  1280x1024 (div 3)");
  Serial.println("  6   VGA  1280x1024 (div 4)\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_scanlines_mode_menu()
{
  Serial.println("\n      * Scanlines mode *\n");

  Serial.println("  s   change scanlines mode\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_buffering_mode_menu()
{
  Serial.println("\n      * Buffering mode *\n");

  Serial.println("  b   change buffering mode\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_cap_sync_mode_menu()
{
  Serial.println("\n      * Capture synchronization source *\n");

  Serial.println("  1   self-synchronizing");
  Serial.println("  2   external clock\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_capture_frequency_menu()
{
  Serial.println("\n      * Capture frequency *\n");

  Serial.println("  1   7000000 Hz (ZX Spectrum  48K)");
  Serial.println("  2   7093800 Hz (ZX Spectrum 128K)");
  Serial.println("  3   custom\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_ext_clk_divider_menu()
{
  Serial.println("\n      * External clock divider *\n");

  Serial.println("  a   increment divider (+1)");
  Serial.println("  z   decrement divider (-1)\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_video_sync_mode_menu()
{
  Serial.println("\n      * Video synchronization mode *\n");

  Serial.println("  y   change synchronization mode\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_image_tuning_menu()
{
  Serial.println("\n      * Capture delay and image position *\n");

  Serial.println("  a   increment delay (+1)");
  Serial.println("  z   decrement delay (-1)\n");

  Serial.println("  i   shift image UP");
  Serial.println("  k   shift image DOWN");
  Serial.println("  j   shift image LEFT");
  Serial.println("  l   shift image RIGHT\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_pin_inversion_mask_menu()
{
  Serial.println("\n      * Pin inversion mask *\n");

  Serial.println("  m   set pin inversion mask\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_test_menu()
{
  Serial.println("\n      * Tests *\n");

  Serial.println("  1   draw welcome image (vertical stripes)");
  Serial.println("  2   draw welcome image (horizontal stripes)");
  Serial.println("  3   draw \"NO SIGNAL\" screen");
  Serial.println("  i   show captured frame count\n");

  Serial.println("  p   show configuration");
  Serial.println("  h   show help (this menu)");
  Serial.println("  q   exit to main menu\n");
}

void print_video_out_mode()
{
  Serial.print("  Video output mode ........... ");
  switch (settings.video_out_mode)
  {
  case DVI:
    Serial.println("HDMI 640x480");
    break;

  case VGA640x480:
    Serial.println("VGA 640x480");
    break;

  case VGA800x600:
    Serial.println("VGA 800x600");
    break;

  case VGA1024x768:
    Serial.println("VGA 1024x768");
    break;

  case VGA1280x1024_d3:
    Serial.println("VGA 1280x1024 (div 3)");
    break;

  case VGA1280x1024_d4:
    Serial.println("VGA 1280x1024 (div 4)");
    break;

  default:
    break;
  }
}

void print_scanlines_mode()
{
  Serial.print("  Scanlines ................... ");

  if (settings.scanlines_mode)
    Serial.println("enabled");
  else
    Serial.println("disabled");
}

void print_buffering_mode()
{
  Serial.print("  Buffering mode .............. ");

  if (settings.x3_buffering_mode)
    Serial.println("x3");
  else
    Serial.println("x1");
}

void print_cap_sync_mode()
{
  Serial.print("  Capture sync source ......... ");
  switch (settings.cap_sync_mode)
  {
  case SELF:
    Serial.println("self-synchronizing");
    break;

  case EXT:
    Serial.println("external clock");
    break;

  default:
    break;
  }
}

void print_capture_frequency()
{
  Serial.print("  Capture frequency ........... ");
  Serial.print(settings.frequency, DEC);
  Serial.println(" Hz");
}

void print_ext_clk_divider()
{
  Serial.print("  External clock divider ...... ");
  Serial.println(settings.ext_clk_divider, DEC);
}

void print_capture_delay()
{
  Serial.print("  Capture delay ............... ");
  Serial.println(settings.delay, DEC);
}

void print_x_offset()
{
  Serial.print("  X offset .................... ");
  Serial.println(settings.shX, DEC);
}

void print_y_offset()
{
  Serial.print("  Y offset .................... ");
  Serial.println(settings.shY, DEC);
}

void print_dividers()
{
  uint16_t div_int;
  uint8_t div_frac;

  video_mode_t video_mode = *(vga_modes[settings.video_out_mode]);

  Serial.print("\n  System clock frequency ...... ");
  Serial.print(clock_get_hz(clk_sys), 1);
  Serial.println(" Hz");

  Serial.print("  Capture divider ............. ");

  pio_calculate_clkdiv_from_float((float)clock_get_hz(clk_sys) / (settings.frequency * 12.0), &div_int, &div_frac);

  Serial.print((div_int + (float)div_frac / 256), 8);

  Serial.print(" ( ");
  Serial.print("0x");
  print_byte_hex((uint8_t)(div_int >> 8));
  print_byte_hex((uint8_t)(div_int & 0xff));
  print_byte_hex(div_frac);
  Serial.println(" )");

  Serial.print("  Video output clock divider .. ");

  pio_calculate_clkdiv_from_float(((float)clock_get_hz(clk_sys) * video_mode.div) / video_mode.pixel_freq, &div_int, &div_frac);

  Serial.print((div_int + (float)div_frac / 256), 8);

  Serial.print(" ( ");
  Serial.print("0x");
  print_byte_hex((uint8_t)(div_int >> 8));
  print_byte_hex((uint8_t)(div_int & 0xff));
  print_byte_hex(div_frac);
  Serial.println(" )\n");
}

void print_video_sync_mode()
{
  Serial.print("  Video synchronization mode .. ");
  if (settings.video_sync_mode)
    Serial.println("separate");
  else
    Serial.println("composite");
}

void print_pin_inversion_mask()
{
  Serial.print("  Pin inversion mask .......... ");
  Serial.println(binary_to_string(settings.pin_inversion_mask, false));
}

void print_settings()
{
  Serial.println("");
  print_video_out_mode();
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
  Serial.println("");
}

void set_scanlines_mode()
{
  if (settings.video_out_mode != DVI)
    set_vga_scanlines_mode(settings.scanlines_mode);
}

void setup()
{
  Serial.begin(9600);

  // loading saved settings
  memcpy(&settings, saved_settings, sizeof(settings_t));
  // correct if there is garbage in the cells
  check_settings(&settings);

  set_v_buf_buffering_mode(settings.x3_buffering_mode);

  draw_welcome_screen(*(vga_modes[settings.video_out_mode]));

  set_scanlines_mode();

  if (settings.video_out_mode == DVI)
  {
    start_dvi(*(vga_modes[settings.video_out_mode]));
  }
  else
  {
    start_vga(*(vga_modes[settings.video_out_mode]));
  }

  start_core0 = true;

  Serial.println("  Starting...\n");
}

void loop()
{
  char inbyte = 0;

  while (1)
  {
    sleep_ms(500);

    if (Serial.available())
    {
      inbyte = 'h';
      break;
    }
  }

  Serial.println(" Entering the configuration mode\n");

  while (1)
  {
    sleep_ms(10);

    if (inbyte != 'h' && Serial.available())
      inbyte = Serial.read();

    switch (inbyte)
    {
    case 'p':
      print_settings();
      inbyte = 0;
      break;

    case 'v':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        uint8_t video_out_mode = settings.video_out_mode;

        switch (inbyte)
        {
        case 'p':
          print_video_out_mode();
          break;

        case 'h':
          print_video_out_menu();
          break;

        case '1':
          settings.video_out_mode = DVI;
          print_video_out_mode();
          break;

        case '2':
          settings.video_out_mode = VGA640x480;
          print_video_out_mode();
          break;

        case '3':
          settings.video_out_mode = VGA800x600;
          print_video_out_mode();
          break;

        case '4':
          settings.video_out_mode = VGA1024x768;
          print_video_out_mode();
          break;

        case '5':
          settings.video_out_mode = VGA1280x1024_d3;
          print_video_out_mode();
          break;
        case '6':
          settings.video_out_mode = VGA1280x1024_d4;
          print_video_out_mode();
          break;

        default:
          break;
        }

        if ((video_out_mode != DVI) && (settings.video_out_mode != DVI) && (video_out_mode != settings.video_out_mode))
        {
          stop_vga();
          start_vga(*(vga_modes[settings.video_out_mode]));
          // capture PIO clock divider needs to be adjusted for new system clock frequency set in start_vga()
          set_capture_frequency(settings.frequency);
        }

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 's':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        switch (inbyte)
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

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'b':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        switch (inbyte)
        {
        case 'p':
          print_buffering_mode();
          break;

        case 'h':
          print_buffering_mode_menu();
          break;

        case 'b':
          settings.x3_buffering_mode = !settings.x3_buffering_mode;
          print_buffering_mode();
          set_v_buf_buffering_mode(settings.x3_buffering_mode);
          break;

        default:
          break;
        }

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'c':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        uint8_t cap_sync_mode = settings.cap_sync_mode;

        switch (inbyte)
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

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'f':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        uint32_t frequency = settings.frequency;

        switch (inbyte)
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
          String frequency_str = "";
          uint32_t frequency_int = 0;

          Serial.print("  Enter frequency: ");

          while (1)
          {
            sleep_ms(10);
            inbyte = 0;

            if (Serial.available())
              inbyte = Serial.read();

            if (inbyte >= '0' && inbyte <= '9')
            {
              Serial.print(inbyte);
              frequency_str += inbyte;
            }

            if (inbyte == '\r')
            {
              Serial.println("");
              frequency_int = string_to_int(frequency_str);

              if (frequency_int >= FREQUENCY_MIN && frequency_int <= FREQUENCY_MAX)
              {
                settings.frequency = frequency_int;
                print_capture_frequency();
                break;
              }
              else
              {
                frequency_str = "";
                Serial.print("  Allowed frequency range ..... ");
                Serial.print(FREQUENCY_MIN, DEC);
                Serial.print(" - ");
                Serial.print(FREQUENCY_MAX, DEC);
                Serial.println(" Hz");
                Serial.print("  Enter frequency: ");
              }
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
          // restart VGA with new capture frequency value which is used to calculate horizontal margins for some video output modes
          if (settings.video_out_mode != DVI)
          {
            stop_vga();
            start_vga(*(vga_modes[settings.video_out_mode]));
          }
        }

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'd':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        switch (inbyte)
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

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'y':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        switch (inbyte)
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

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 't':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        switch (inbyte)
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

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'm':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        uint8_t pin_inversion_mask = settings.pin_inversion_mask;

        switch (inbyte)
        {
        case 'p':
          print_pin_inversion_mask();
          break;

        case 'h':
          print_pin_inversion_mask_menu();
          break;

        case 'm':
        {
          String pin_inversion_mask_str = "";

          Serial.print("  Enter pin inversion mask: ");

          while (1)
          {
            sleep_ms(10);
            inbyte = 0;

            if (Serial.available())
              inbyte = Serial.read();

            if (inbyte >= '0' && inbyte <= '1')
            {
              Serial.print(inbyte);
              pin_inversion_mask_str += inbyte;
            }

            if (inbyte == '\r')
            {
              Serial.println("");

              uint8_t pin_inversion_mask_int = 0;

              for (int i = 0; i < pin_inversion_mask_str.length(); i++)
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
              else
              {
                pin_inversion_mask_str = "";
                Serial.print("  Allowed inversion mask ...... ");
                Serial.println(binary_to_string(PIN_INVERSION_MASK, true));
                Serial.print("  Enter pin inversion mask: ");
              }
            }
          }

          break;
        }

        default:
          break;
        }

        if (pin_inversion_mask != settings.pin_inversion_mask)
          set_pin_inversion_mask(settings.pin_inversion_mask);

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'T':
    {
      inbyte = 'h';

      while (1)
      {
        sleep_ms(10);

        if (inbyte != 'h' && Serial.available())
          inbyte = Serial.read();

        switch (inbyte)
        {
        case 'p':
          print_settings();
          break;

        case 'h':
          print_test_menu();
          break;

        case 'i':
          Serial.print("  Current frame count ......... ");
          Serial.println(frame_count - 1, DEC);
          break;

        case '1':
        case '2':
        case '3':
        {
          uint32_t frame_count_tmp = frame_count;

          sleep_ms(100);

          if (frame_count == frame_count_tmp) // draw the screen only if capture is not active
          {
            Serial.println("  Drawing the screen...");

            if (inbyte == '1')
              draw_welcome_screen(*(vga_modes[settings.video_out_mode]));
            else if (inbyte == '2')
              draw_welcome_screen_h(*(vga_modes[settings.video_out_mode]));
            else
              draw_no_signal(*(vga_modes[settings.video_out_mode]));
          }

          break;
        }

        default:
          break;
        }

        if (inbyte == 'q')
        {
          inbyte = 'h';
          break;
        }

        inbyte = 0;
      }

      break;
    }

    case 'h':
      print_main_menu();
      inbyte = 0;
      break;

    case 'w':
      save_settings(&settings);
      inbyte = 0;
      break;

    case 'r':
      watchdog_reboot(0, 0, 0);
      break;

    default:
      break;
    }

    if (inbyte == 'q')
    {
      inbyte = 0;

      Serial.println(" Leaving the configuration mode\n");

      break;
    }
  }
}

void __attribute__((weak)) setup1()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  while (!start_core0)
    sleep_ms(10);

  start_capture(&settings);
}

void __attribute__((weak)) __not_in_flash_func(loop1())
{
  uint32_t frame_count_tmp1 = frame_count;

  sleep_ms(100);

  if (frame_count > 1)
  {
    digitalWrite(PIN_LED, (frame_count & 0x20) && capture_active);

    if (frame_count == frame_count_tmp1)
    {
      if (capture_active)
      {
        capture_active = false;
        draw_no_signal(*(vga_modes[settings.video_out_mode]));
      }
    }
    else if (!capture_active)
      capture_active = true;
  }

  if (restart_capture)
  {
    stop_capture();
    start_capture(&settings);
    restart_capture = false;
  }

  if (stop_core1)
  {
    core1_inactive = true;

    uint32_t ints = save_and_disable_interrupts();

    while (core1_inactive)
      ;

    restore_interrupts_from_disabled(ints);
  }
}

void main1()
{
  if (setup1)
    setup1();

  while (1)
    if (loop1)
      loop1();
}

int main()
{
  multicore_launch_core1(main1);

  setup();

  while (1)
    loop();
}
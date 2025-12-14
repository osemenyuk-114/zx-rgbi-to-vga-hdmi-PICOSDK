#include <stdio.h>
#include <string>
#include <cstring>

#include "pico.h"
#include "pico/time.h"
#include "hardware/vreg.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/clocks.h"

#include "serial_menu.h"

extern "C"
{
#include "g_config.h"
#include "rgb_capture.h"
#include "settings.h"
#include "v_buf.h"
#include "video_output.h"

#ifdef OSD_ENABLE
#include "osd.h"
#endif

#ifdef OSD_MENU_ENABLE
#include "osd_menu.h"
#endif

#ifdef OSD_FF_ENABLE
#include "ff_osd_i2c.h"
#endif
}

#define PIN_LED (25u)

settings_t settings;
video_mode_t video_mode;

volatile bool start_core0 = false;

volatile bool stop_core1 = false;
volatile bool core1_inactive = false;

volatile bool restart_capture = false;
volatile bool capture_active = false;

// Video output active state flag
video_out_type_t active_video_output = VIDEO_OUT_TYPE_DEF;

void setup()
{
  vreg_set_voltage(VREG_VOLTAGE_1_25);
  sleep_ms(100);

  stdio_init_all();

  load_settings(&settings);
  set_buffering_mode(settings.buffering_mode);
  draw_welcome_screen(*(video_modes[settings.video_out_mode]));
  set_scanlines_mode();
  start_video_output(settings.video_out_type);

#ifdef OSD_ENABLE
  osd_init();
#endif

  start_core0 = true;

  printf("  Starting...\n\n");
}

void loop()
{
#ifdef OSD_ENABLE
  osd_update();

  if (!osd_state.visible)
  {
#endif
    char c = get_menu_input(100);

    if (c != 0)
      handle_serial_menu();

#ifdef OSD_ENABLE
  }
#endif
}

void __attribute__((weak)) setup1()
{
  gpio_init(PIN_LED);
  gpio_set_dir(PIN_LED, GPIO_OUT);
  gpio_put(PIN_LED, 0);

#ifdef OSD_FF_ENABLE
  ff_osd_i2c_init();
#endif

  while (!start_core0)
    sleep_ms(10);

  start_capture();
}

void __attribute__((weak)) __not_in_flash_func(loop1())
{
  uint32_t frame_count_tmp1 = frame_count;

#ifdef OSD_FF_ENABLE
  //  Call osd_process frequently to keep up with I2C data
  for (int i = 0; i < 100; i++)
  {
    ff_osd_i2c_process();
    sleep_ms(1);
  }
#else
  sleep_ms(100);
#endif

  if (frame_count > 1)
  {
    gpio_put(PIN_LED, (frame_count & 0x20) && capture_active);

    if (frame_count == frame_count_tmp1)
    {
      if (capture_active)
      {
        capture_active = false;
        draw_no_signal(*(video_modes[settings.video_out_mode]));
      }
    }
    else if (!capture_active)
      capture_active = true;
  }

  if (restart_capture)
  {
    stop_capture();
    start_capture();
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
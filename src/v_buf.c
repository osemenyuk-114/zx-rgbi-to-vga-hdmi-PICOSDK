#include "g_config.h"
#include "v_buf.h"

extern settings_t settings;

#ifdef HIRES_ENABLE
uint8_t *v_bufs = g_v_buf;
#else
uint8_t *v_bufs[3] = {g_v_buf, g_v_buf + V_BUF_SZ, g_v_buf + (2 * V_BUF_SZ)};

bool show_v_buf[] = {false, false, false};

uint8_t v_buf_in_idx = 0;
uint8_t v_buf_out_idx = 0;

bool buffering_mode = false;
bool first_frame = true;
#endif

void *__not_in_flash_func(get_v_buf_out)()
{
#ifdef HIRES_ENABLE
  return v_bufs;
#else
  if (!buffering_mode || first_frame)
    return v_bufs[0];

  if (!show_v_buf[(v_buf_out_idx + 1) % 3])
  {
    show_v_buf[v_buf_out_idx] = true;
    v_buf_out_idx = (v_buf_out_idx + 1) % 3;
    return v_bufs[v_buf_out_idx];
  }

  if (!show_v_buf[(v_buf_out_idx + 2) % 3])
  {
    show_v_buf[v_buf_out_idx] = true;
    v_buf_out_idx = (v_buf_out_idx + 2) % 3;
    return v_bufs[v_buf_out_idx];
  }

  return v_bufs[v_buf_out_idx];
#endif
}

void *__not_in_flash_func(get_v_buf_in)()
{
#ifdef HIRES_ENABLE
  return v_bufs;
#else
  if (!buffering_mode)
    return v_bufs[0];

  first_frame = false;
  show_v_buf[v_buf_in_idx] = false;

  if (show_v_buf[(v_buf_in_idx + 1) % 3])
  {
    v_buf_in_idx = (v_buf_in_idx + 1) % 3;
    return v_bufs[v_buf_in_idx];
  }

  if (show_v_buf[(v_buf_in_idx + 2) % 3])
  {
    v_buf_in_idx = (v_buf_in_idx + 2) % 3;
    return v_bufs[v_buf_in_idx];
  }

  return NULL;
#endif
}
#ifndef HIRES_ENABLE
void set_buffering_mode(bool buf_mode)
{
  buffering_mode = buf_mode;
}
#endif
void clear_video_buffers()
{
  // clear all three video buffers
  memset(g_v_buf, 0, V_BUF_SZ * V_BUF_NUM);

#ifndef HIRES_ENABLE
  // reset buffer indices and flags
  v_buf_in_idx = 0;
  v_buf_out_idx = 0;
  show_v_buf[0] = false;
  show_v_buf[1] = false;
  show_v_buf[2] = false;
  first_frame = true;
#endif
}
#include "g_config.h"
#include "v_buf.h"

extern settings_t settings;

uint8_t *v_bufs[3] = {g_v_buf, g_v_buf + V_BUF_SZ, g_v_buf + 2 * V_BUF_SZ};

bool show_v_buf[] = {false, false, false};

uint8_t v_buf_in_idx = 0;
uint8_t v_buf_out_idx = 0;

bool x3_buffering_mode = false;
bool first_frame = true;

void *__not_in_flash_func(get_v_buf_out)()
{
  if (!x3_buffering_mode | first_frame)
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
}

void *__not_in_flash_func(get_v_buf_in)()
{
  if (!x3_buffering_mode)
    return v_bufs[0];

  if (first_frame)
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
}

void set_v_buf_buffering_mode(bool buffering_mode)
{
  x3_buffering_mode = buffering_mode;
}

void draw_welcome_screen(video_mode_t video_mode)
{
  int16_t h_visible_area = (uint16_t)(video_mode.h_visible_area / (video_mode.div * 4)) * 4;
  int16_t h_margin = h_visible_area - (uint8_t)(settings.frequency / 1000000) * ACTIVE_VIDEO_TIME;

  if (h_margin < 0)
    h_margin = 0;

  h_visible_area -= h_margin;

  uint8_t *v_buf = (uint8_t *)get_v_buf_out();

  for (int y = 0; y < V_BUF_H; y++)
    for (int x = 0; x < V_BUF_W; x++)
    {
      uint8_t i = 0x0f & ~(uint8_t)((16 * x) / h_visible_area);
      uint8_t R = (i & 4) ? ((i & 1) ? 0b0100 : 0b1100) : 0;
      uint8_t G = (i & 8) ? ((i & 1) ? 0b0010 : 0b1010) : 0;
      uint8_t B = (i & 2) ? ((i & 1) ? 0b0001 : 0b1001) : 0;
      uint8_t c = R | G | B;

      if (x & 1)
        *v_buf++ |= c << 4;
      else
        *v_buf = c & 0x0f;
    }
}

void draw_welcome_screen_h(video_mode_t video_mode)
{
  int16_t v_margin = ((int16_t)((video_mode.v_visible_area - V_BUF_H * video_mode.div) / (video_mode.div * 2) + 0.5)) * video_mode.div * 2;

  if (v_margin < 0)
    v_margin = 0;

  uint16_t v_visible_area = video_mode.v_visible_area - v_margin;

  uint8_t *v_buf = (uint8_t *)get_v_buf_out();

  for (int y = 0; y < V_BUF_H; y++)
  {
    uint8_t i = (16 * y * video_mode.div) / v_visible_area;
    uint8_t R = (i & 4) ? ((i & 1) ? 0b0100 : 0b1100) : 0;
    uint8_t G = (i & 8) ? ((i & 1) ? 0b0010 : 0b1010) : 0;
    uint8_t B = (i & 2) ? ((i & 1) ? 0b0001 : 0b1001) : 0;
    uint8_t c = R | G | B;

    for (int x = 0; x < V_BUF_W / 2; x++)
    {
      c |= c << 4;
      *v_buf++ = c;
    }
  }
}

const char nosignal[14][114] = {
    "xx      xx      xxxxxx                  xxxxxx      xxxxxx      xxxxxx      xx      xx        xx        xx",
    "xx      xx     xxxxxxxx                xxxxxxxx     xxxxxx     xxxxxxxx     xx      xx       xxxx       xx",
    "xxx     xx    xxx    xxx              xxx    xxx      xx      xxx    xxx    xxx     xx      xxxxxx      xx",
    "xxx     xx    xx      xx              xx      xx      xx      xx      xx    xxx     xx     xxx  xxx     xx",
    "xxxx    xx    xx      xx              xx              xx      xx            xxxx    xx    xxx    xxx    xx",
    "xxxxx   xx    xx      xx              xxx             xx      xx            xxxxx   xx    xx      xx    xx",
    "xx xxx  xx    xx      xx               xxxxxxx        xx      xx            xx xxx  xx    xx      xx    xx",
    "xx  xxx xx    xx      xx                xxxxxxx       xx      xx    xxxx    xx  xxx xx    xx      xx    xx",
    "xx   xxxxx    xx      xx                     xxx      xx      xx    xxxx    xx   xxxxx    xxxxxxxxxx    xx",
    "xx    xxxx    xx      xx                      xx      xx      xx      xx    xx    xxxx    xxxxxxxxxx    xx",
    "xx     xxx    xx      xx              xx      xx      xx      xx      xx    xx     xxx    xx      xx    xx",
    "xx     xxx    xxx    xxx              xxx    xxx      xx      xxx    xxx    xx     xxx    xx      xx    xx",
    "xx      xx     xxxxxxxx                xxxxxxxx     xxxxxx     xxxxxxxx     xx      xx    xx      xx    xxxxxxxxxx",
    "xx      xx      xxxxxx                  xxxxxx      xxxxxx      xxxxxx      xx      xx    xx      xx    xxxxxxxxxx"};

void draw_no_signal(video_mode_t video_mode)
{
  uint8_t c;
  uint8_t c2;

  int16_t h_visible_area = (uint16_t)(video_mode.h_visible_area / (video_mode.div * 4)) * 4;
  int16_t h_margin = h_visible_area - (uint8_t)(settings.frequency / 1000000) * ACTIVE_VIDEO_TIME;

  if (h_margin < 0)
    h_margin = 0;

  int16_t v_margin = ((int16_t)((video_mode.v_visible_area - V_BUF_H * video_mode.div) / (video_mode.div * 2) + 0.5)) * video_mode.div * 2;

  if (v_margin < 0)
    v_margin = 0;

  uint16_t y = (video_mode.v_visible_area - v_margin) / (video_mode.div * 2);
  uint16_t x = (h_visible_area - h_margin - 114) / 4;

  uint8_t *v_buf = (uint8_t *)get_v_buf_out();

  memset(v_buf, 0, V_BUF_H * V_BUF_W / 2);

  for (int row = 0; row < 14; ++row)
    for (int col = 0; col < 114; ++col)
    {
      c = (nosignal[row][col] == 'x') ? 0b0111 : 0b0000;

      if (col & 1)
        v_buf[(y + row) * V_BUF_W / 2 + x + col / 2] = c2 | (c << 4);
      else
        c2 = c;
    }
}

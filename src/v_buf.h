#pragma once

void *get_v_buf_out();
void *get_v_buf_in();
#ifndef HIRES_ENABLE
void set_buffering_mode(bool);
#endif
void clear_video_buffers();
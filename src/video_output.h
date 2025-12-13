#pragma once

void start_video_output(video_out_type_t);
void stop_video_output();
void set_scanlines_mode();
void set_osd_position(uint8_t position);
void draw_welcome_screen(video_mode_t);
void draw_welcome_screen_h(video_mode_t);
void draw_no_signal(video_mode_t);
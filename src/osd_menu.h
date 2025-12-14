#pragma once

#include "osd.h"

// OSD Menu-specific definitions
#define OSD_BTN_UP 26
#define OSD_BTN_DOWN 27
#define OSD_BTN_SEL 28

#define OSD_MENU_TIMEOUT_US 10000000 // 10 seconds

// Menu layout constants
#define OSD_TITLE_ROW 1
#define OSD_SUBTITLE_ROW 3
#define OSD_MENU_START_ROW 5

#define MENU_TYPE_MAIN 0
#define MENU_TYPE_OUTPUT 1
#define MENU_TYPE_CAPTURE 2
#define MENU_TYPE_IMAGE_ADJUST 3
#define MENU_TYPE_MASK 4
#define MENU_TYPE_ABOUT 5

typedef enum
{
    MENU_ITEM_SUBMENU, // Opens a submenu
    MENU_ITEM_TOGGLE,  // Toggle parameter (bool or enum)
    MENU_ITEM_RANGE,   // Adjustable range parameter
    MENU_ITEM_ACTION,  // Executes an action
    MENU_ITEM_BACK     // Returns to previous menu
} menu_item_type_t;

// Menu-specific OSD state extension
typedef struct
{
    uint8_t selected_item;              // Currently selected menu item
    bool tuning_mode;                   // True when adjusting parameters, false for navigation
    uint8_t original_video_mode;        // Store original mode when entering video mode tuning
    uint8_t mask_bit_position;          // Currently selected bit position for mask editing (0-6, bit 7 always 0)
    uint8_t mask_last_toggled_position; // Last bit position that was toggled (for exit detection)
} osd_menu_state_t;

typedef struct
{
    bool up_pressed;
    bool down_pressed;
    bool sel_pressed;
    uint32_t last_press_time[3]; // Debounce timing
    bool repeat_enabled;
    uint32_t key_hold_start[3];    // When key hold started
    uint32_t last_repeat_time[3];  // Last repeat trigger time
    bool key_held[3];              // Is key currently held down
    bool sel_long_press_triggered; // True if SEL long press (>5s) has been triggered
} osd_buttons_t;

typedef struct
{
    uint8_t current_menu;
    uint8_t menu_depth;
    uint8_t menu_stack[4]; // Support 4 levels of nested menus
    uint8_t item_stack[4];
} osd_menu_nav_t;

extern osd_buttons_t osd_buttons;
extern osd_menu_nav_t osd_menu;
extern osd_menu_state_t osd_menu_state;

// Menu functions
void osd_menu_init();
void osd_menu_update();
void osd_menu_toggle();

void osd_buttons_init();
void osd_buttons_update();
bool osd_button_pressed(uint8_t button);

void osd_update_text_buffer(); // Update text buffer based on current menu state

void osd_adjust_image_parameter(uint8_t param_index, int8_t direction);
void osd_adjust_video_mode(int8_t direction);
void osd_adjust_capture_parameter(uint8_t param_index, int8_t direction);

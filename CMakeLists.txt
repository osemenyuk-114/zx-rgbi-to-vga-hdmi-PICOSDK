# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

# Allow FetchContent_Populate for fine-grained dependency control
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_NAME "ZX_RGBI_TO_VGA_HDMI")

# =============================================================================
# CONFIGURATION — edit these settings or pass via -D on the command line
# =============================================================================

# Board variant (pass -DBOARD=BOARD_XXX to override)
# Supported: BOARD_36LJU22, BOARD_38LJE24, BOARD_RP2040ZERO
#            BOARD_11XGA24_1, BOARD_11XGA24_2,
#            BOARD_25LEO25, BOARD_09LJV23
set(BOARD "BOARD_38LJE24" CACHE STRING "Board variant")

# Set to ON for Raspberry Pi Pico 2 (RP2350), OFF for original Pico (RP2040)
option(USE_PICO2 "Build for Raspberry Pi Pico 2 (RP2350)" OFF)

# Feature toggles
set(OSD_MENU_ENABLE ON CACHE BOOL "Enable OSD menu")
set(OSD_FF_ENABLE ON CACHE BOOL "Enable FlashFloppy I2C OSD")
set(SERIAL_MENU_ENABLE ON CACHE BOOL "Enable serial terminal menu")

# Keyboard support
set(PS2_KBD_ENABLE ON CACHE BOOL "Enable PS/2 keyboard")
set(USB_KBD_ENABLE ON CACHE BOOL "Enable USB keyboard")

# =============================================================================
# END OF CONFIGURATION
# =============================================================================

# Force USE_PICO2 OFF for RP2040-based boards (no RP2350 variant)
if(BOARD STREQUAL "BOARD_RP2040ZERO")
    set(USE_PICO2 OFF CACHE BOOL "Force USE_PICO2 OFF for RP2040ZERO" FORCE)
endif()

# Select Pico board type based on board variant (FORCE overrides cached value)
if(USE_PICO2)
    set(PICO_BOARD pico2 CACHE STRING "Board type" FORCE)
    set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Target platform" FORCE)
else()  
    set(PICO_BOARD pico CACHE STRING "Board type" FORCE)
    set(PICO_PLATFORM rp2040 CACHE STRING "Target platform" FORCE)
endif()

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(FetchContent)

project(ZX_RGBI_TO_VGA_HDMI C CXX ASM)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Pass board definition to compiler
add_compile_definitions(${BOARD})

# Force keyboard support OFF for RP2040ZERO and 11XGA24 boards due to pin conflicts with OSD and/or lack of available pins
if(BOARD STREQUAL "BOARD_RP2040ZERO" OR BOARD STREQUAL "BOARD_11XGA24_1" OR BOARD STREQUAL "BOARD_11XGA24_2")
    set(PS2_KBD_ENABLE OFF CACHE BOOL "Force PS/2 keyboard OFF for RP2040ZERO and 11XGA24 boards" FORCE)
    set(USB_KBD_ENABLE OFF CACHE BOOL "Force USB keyboard OFF for RP2040ZERO and 11XGA24 boards" FORCE)
endif()

# Force OSD_FF_ENABLE OFF for 11XGA24 board
if(BOARD STREQUAL "BOARD_11XGA24_1" OR BOARD STREQUAL "BOARD_11XGA24_2")
    set(OSD_FF_ENABLE OFF CACHE BOOL "Force FlashFloppy I2C OSD OFF for 11XGA24" FORCE)
endif()

# Force SERIAL_MENU_ENABLE ON when OSD_MENU_ENABLE is OFF
if(NOT OSD_MENU_ENABLE)
    set(SERIAL_MENU_ENABLE ON)
endif()

# USB Host mode and USB Device stdio are mutually exclusive
if(USB_KBD_ENABLE)
    set(SERIAL_MENU_ENABLE OFF)
endif()

# Automatically fetch pico_i2c_slave when FlashFloppy OSD is enabled.
# Only add the i2c_slave library subdirectory, skip examples.
if(OSD_FF_ENABLE)
    # Use modern FetchContent pattern while excluding example subdirectories
    FetchContent_Declare(
        pico_i2c_slave
        GIT_REPOSITORY https://github.com/vmilea/pico_i2c_slave.git
        GIT_TAG master
    )
    FetchContent_GetProperties(pico_i2c_slave)
    if(NOT pico_i2c_slave_POPULATED)
        FetchContent_Populate(pico_i2c_slave)
        # Only add the i2c_slave library, not the examples
        add_subdirectory(
            ${pico_i2c_slave_SOURCE_DIR}/i2c_slave
            ${pico_i2c_slave_BINARY_DIR}/i2c_slave
            EXCLUDE_FROM_ALL
        )
    endif()
endif()

# Add executable
add_executable(${EXECUTABLE_NAME})

target_sources(
    ${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/g_config.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/settings.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/video/dvi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/video/rgb_capture.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/video/v_buf.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/video/vga.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/video/video_output.c
)

# Conditionally add serial menu (requires USB stdio)
if(SERIAL_MENU_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/serial_menu.c
    )
endif()

# Conditionally add OSD common code (used by both MENU and FF)
if(OSD_MENU_ENABLE OR OSD_FF_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/osd/osd.c
    )
endif()

# Conditionally add OSD menu source files
if(OSD_MENU_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/osd/osd_menu.c
    )
endif()

# Conditionally add FlashFloppy OSD source files
if(OSD_FF_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/osd/ff_osd.c
    )
endif()

# Conditionally add keyboard source files
if(PS2_KBD_ENABLE OR USB_KBD_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/kbd.c
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/osd_kbd.c
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/zx_kbd.c
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/ch446q.c
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/epm3256.c
    )
endif()

if(PS2_KBD_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/ps2_kbd.c
    )
endif()

if(USB_KBD_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/kbd/usb_kbd.c
    )
endif()

# Generate PIO header
pico_generate_pio_header(${EXECUTABLE_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/video/video.pio
    ${CMAKE_CURRENT_LIST_DIR}/src/kbd/kbd.pio
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${EXECUTABLE_NAME} 0)
if(USB_KBD_ENABLE)
    # USB port is in host mode — disable device-mode stdio entirely
    pico_enable_stdio_usb(${EXECUTABLE_NAME} 0)
else()
    pico_enable_stdio_usb(${EXECUTABLE_NAME} $<BOOL:${SERIAL_MENU_ENABLE}>)
endif()

# Add the standard include files to the build
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/video
    ${CMAKE_CURRENT_LIST_DIR}/src/osd
    ${CMAKE_CURRENT_LIST_DIR}/src/kbd
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
)

# Add any user requested libraries
target_link_libraries(${EXECUTABLE_NAME}
    hardware_pio 
    hardware_dma 
    hardware_flash 
    pico_multicore 
    pico_unique_id  
    pico_stdlib
)

# Link USB stdio library for headers when not in USB host mode
if(NOT SERIAL_MENU_ENABLE AND NOT USB_KBD_ENABLE)
    target_link_libraries(${EXECUTABLE_NAME} pico_stdio_usb)
endif()

# Conditionally link TinyUSB host library for USB keyboard
if(USB_KBD_ENABLE)
    target_link_libraries(${EXECUTABLE_NAME}
        tinyusb_host
        tinyusb_board
    )
endif()

# Conditionally link I2C libraries for FlashFloppy OSD
if(OSD_FF_ENABLE)
    target_link_libraries(${EXECUTABLE_NAME}
        hardware_i2c
        pico_i2c_slave
    )
endif()

target_compile_options(${EXECUTABLE_NAME} PRIVATE 
    -O3 
)

# Add compile definitions for OSD features
if(OSD_MENU_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE OSD_MENU_ENABLE)
endif()

if(OSD_FF_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE OSD_FF_ENABLE)
endif()

# Define SERIAL_MENU_ENABLE for source code when option is enabled
if(SERIAL_MENU_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE SERIAL_MENU_ENABLE)
endif()

# Keyboard compile definitions
if(PS2_KBD_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE PS2_KBD_ENABLE)
endif()

if(USB_KBD_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
        USB_KBD_ENABLE
        NO_USB
        CFG_TUSB_CONFIG_FILE="tusb_config_host.h"
    )
endif()

# Add linker flag to print memory usage
target_link_options(${EXECUTABLE_NAME} PRIVATE
    -Wl,--print-memory-usage
)

# --- Output filename: ZX_RGBI_TO_VGA_HDMI-<board>[-PICO2]-<version> ---
string(REPLACE "BOARD_" "" BOARD_SHORT "${BOARD}")
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/src/g_config.h" _fw_ver_line REGEX "#define FW_VER ")
string(REGEX MATCH "\"([^\"]+)\"" _ "${_fw_ver_line}")
string(REGEX REPLACE "^v" "" FW_VER_NO_V "${CMAKE_MATCH_1}")

if(USE_PICO2)
    set(FW_OUTPUT_NAME "ZX_RGBI_TO_VGA_HDMI-${BOARD_SHORT}-PICO2-${FW_VER_NO_V}")
else()
    set(FW_OUTPUT_NAME "ZX_RGBI_TO_VGA_HDMI-${BOARD_SHORT}-${FW_VER_NO_V}")
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME "${FW_OUTPUT_NAME}")
message(STATUS "Output firmware: ${FW_OUTPUT_NAME}.uf2")

pico_add_extra_outputs(${EXECUTABLE_NAME})

# Extend the clean target to remove all object files (*.dir subdirectories only)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CMAKE_BINARY_DIR}/CMakeFiles/${EXECUTABLE_NAME}.dir"
)

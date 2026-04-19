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
set(PICO_BOARD pico CACHE STRING "Board type")

# Initialize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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

# Build options — override via CMake presets or -D flags
# These control which source files are compiled and which libraries are linked

# Board variant — uncomment exactly one
set(BOARD "BOARD_36LJU22")
# set(BOARD "BOARD_38LJE24")
# set(BOARD "BOARD_11XGA24")
# set(BOARD "BOARD_25LEO25")
# set(BOARD "BOARD_09LJV23")

# Pass board definition to compiler
add_compile_definitions(${BOARD})

# Build options for OSD features — cache variables to support CMakePresets
# To change: use presets (cmake --preset no-osd) OR edit here + clear cache
set(OSD_MENU_ENABLE ON CACHE BOOL "Enable OSD settings menu")
set(OSD_FF_ENABLE ON CACHE BOOL "Enable FlashFloppy I2C OSD")

# Serial menu option — NOT cached, changes take effect immediately
set(SERIAL_MENU_ENABLE ON)

# Force OSD_FF_ENABLE OFF for 11XGA24 board
if(BOARD STREQUAL "BOARD_11XGA24")
    set(OSD_FF_ENABLE OFF CACHE BOOL "Force FlashFloppy I2C OSD OFF for 11XGA24" FORCE)
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
    ${CMAKE_CURRENT_LIST_DIR}/src/dvi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/g_config.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/rgb_capture.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/settings.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/v_buf.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/vga.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/video_output.c
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
        ${CMAKE_CURRENT_LIST_DIR}/src/osd.c
    )
endif()

# Conditionally add OSD menu source files
if(OSD_MENU_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/osd_menu.c
    )
endif()

# Conditionally add FlashFloppy OSD source files
if(OSD_FF_ENABLE)
    target_sources(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/ff_osd.c
    )
endif()

# Generate PIO header
pico_generate_pio_header(${EXECUTABLE_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/programs.pio
)

# Modify the below lines to enable/disable output over UART/USB
# Note: Always enable stdio_usb library linking (for headers) but disable init when not needed
pico_enable_stdio_uart(${EXECUTABLE_NAME} 0)
pico_enable_stdio_usb(${EXECUTABLE_NAME} $<BOOL:${SERIAL_MENU_ENABLE}>)

# Add the standard include files to the build
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
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

# Always link USB stdio library for headers (even when disabled)
# This allows code to include pico/stdio_usb.h without conditional compilation
if(NOT SERIAL_MENU_ENABLE)
    target_link_libraries(${EXECUTABLE_NAME} pico_stdio_usb)
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

# Add firmware version definition if provided via CMake
if(DEFINED FW_VERSION)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE FW_VERSION="${FW_VERSION}")
endif()

# Add linker flag to print memory usage
target_link_options(${EXECUTABLE_NAME} PRIVATE
    -Wl,--print-memory-usage
)

pico_add_extra_outputs(${EXECUTABLE_NAME})

# Extend the clean target to remove all object files (*.dir subdirectories only)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CMAKE_BINARY_DIR}/CMakeFiles/${EXECUTABLE_NAME}.dir"
)

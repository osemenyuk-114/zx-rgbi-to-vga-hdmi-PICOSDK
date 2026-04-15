# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

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
set(toolchainVersion 14_2_Rel1)
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

# Board variant — uncomment exactly one (only one may be active at a time)
set(BOARD_TYPE "36LJU22" CACHE STRING "Board variant")
# set(BOARD_TYPE "11XGA24" CACHE STRING "Board variant")
# set(BOARD_TYPE "LEO_REV3" CACHE STRING "Board variant")
# set(BOARD_TYPE "09LJV23" CACHE STRING "Board variant")

option(OSD_MENU_ENABLE "Enable OSD settings menu" ON)
option(OSD_FF_ENABLE "Enable FlashFloppy I2C OSD" ON)

# Automatically fetch pico_i2c_slave when FlashFloppy OSD is enabled.
# Only add the i2c_slave library subdirectory, skip examples.
if(OSD_FF_ENABLE)
    FetchContent_Declare(
        pico_i2c_slave
        GIT_REPOSITORY https://github.com/vmilea/pico_i2c_slave.git
        GIT_TAG master
    )
    FetchContent_Populate(pico_i2c_slave)
    add_subdirectory(
        ${pico_i2c_slave_SOURCE_DIR}/i2c_slave
        ${pico_i2c_slave_BINARY_DIR}/i2c_slave
        EXCLUDE_FROM_ALL
    )
endif()

# Add executable
add_executable(${EXECUTABLE_NAME})

target_sources(
    ${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/dvi.c
    ${CMAKE_CURRENT_LIST_DIR}/src/g_config.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/rgb_capture.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/serial_menu.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/settings.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/v_buf.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/vga.c 
    ${CMAKE_CURRENT_LIST_DIR}/src/video_output.c
)

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
pico_enable_stdio_uart(${EXECUTABLE_NAME} 0)
pico_enable_stdio_usb(${EXECUTABLE_NAME} 1)

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

# Add compile definitions for board type and OSD features
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE BOARD_${BOARD_TYPE})

if(OSD_MENU_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE OSD_MENU_ENABLE)
endif()

if(OSD_FF_ENABLE)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE OSD_FF_ENABLE)
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

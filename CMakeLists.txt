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

project(ZX_RGBI_TO_VGA_HDMI C CXX ASM)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(${EXECUTABLE_NAME} dvi.c g_config.c main.cpp rgb_capture.c serial_menu.cpp settings.c v_buf.c vga.c video_output.c)

# Generate PIO header
pico_generate_pio_header(${EXECUTABLE_NAME} ${CMAKE_CURRENT_LIST_DIR}/programs.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${EXECUTABLE_NAME} 0)
pico_enable_stdio_usb(${EXECUTABLE_NAME} 1)

# Add the standard include files to the build
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Add any user requested libraries
target_link_libraries(${EXECUTABLE_NAME} hardware_pio hardware_dma hardware_flash pico_multicore pico_unique_id  pico_stdlib)

target_compile_options(${EXECUTABLE_NAME} PRIVATE -O3 )

# Add linker flag to print memory usage
target_link_options(${EXECUTABLE_NAME} PRIVATE -Wl,--print-memory-usage )

pico_add_extra_outputs(${EXECUTABLE_NAME})

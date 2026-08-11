# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(vgaPico2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1

#add_executable(vgaPico2 vga.c vga_drv.c vga_primitives.c orion_bus.c vga_bus_read.c cursor.c font.c font_8x12.c font_8x14.c font_8x16.c font_8x18.c eeprom.c)
add_executable(vgaPico2 vga.c vga_drv.c vga_primitives.c vga_graphos.c orion_bus.c cursor.c font.c font_8x12.c font_8x14.c font_8x16.c font_8x18.c eeprom.c)

pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga320p_hsync.pio)
pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga320p_vsync.pio)
pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga320p_rgb.pio)
pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga640p_hsync.pio)
pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga640p_vsync.pio)
pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga640p_rgb.pio)
#pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/vga_bus_read.pio)
pico_generate_pio_header(vgaPico2 ${CMAKE_CURRENT_SOURCE_DIR}/orion_bus.pio)


pico_set_program_name(vgaPico2 "vgaPico2")
pico_set_program_version(vgaPico2 "0.1")


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(vgaPico2 0)
pico_enable_stdio_usb(vgaPico2 1)

# Add the standard library to the build
# Add the standard include files to the build
target_include_directories(vgaPico2 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(vgaPico2
    pico_stdlib
    hardware_pio
    hardware_dma
    hardware_gpio
    hardware_irq
    pico_multicore
    m
    hardware_i2c
    hardware_watchdog
)
pico_add_extra_outputs(vgaPico2)


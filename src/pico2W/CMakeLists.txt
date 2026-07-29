# Generated Cmake Pico project file
#
# rm -rf build && mkdir build && cd build
# cmake -DPICO_BOARD=pico2_w ..
#
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
set(PICO_BOARD pico2_w CACHE STRING "pds200")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pds200Pico2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(./FatFs_SPI FatFs_SPI)

# Add executable. Default name is the project name, version 0.1
add_executable(pds200Pico2  pds200.c pds200_1.c orion_bus.c ps2_keyboard.c ringbuffer.c orion_sd_test.c hw_config.c)

pico_generate_pio_header(pds200Pico2 ${CMAKE_CURRENT_SOURCE_DIR}/orion_bus.pio)
#pico_generate_pio_header(pds200Pico2 ${CMAKE_CURRENT_SOURCE_DIR}/orion_bus1.pio)
pico_generate_pio_header(pds200Pico2 ${CMAKE_CURRENT_SOURCE_DIR}/ps2_keyboard.pio)

pico_set_program_name(pds200Pico2 "pds200Pico2")
pico_set_program_version(pds200Pico2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(pds200Pico2 1)
pico_enable_stdio_uart(pds200Pico2 0)

# Add the standard include files to the build
target_include_directories(pds200Pico2 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pds200Pico2
    pico_stdlib
    hardware_pio
    hardware_dma
    hardware_gpio
    hardware_irq
    pico_multicore
    m
    hardware_i2c
    hardware_watchdog
    pico_cyw43_arch_lwip_threadsafe_background    
    FatFs_SPI
)
pico_add_extra_outputs(pds200Pico2)

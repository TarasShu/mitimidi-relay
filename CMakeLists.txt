cmake_minimum_required(VERSION 3.18)

# Set board to Pico W before anything else
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set project name  
project(mitimidi-relay C CXX ASM)

# Initialize the SDK
pico_sdk_init()

# Add subdirectories for BLE MIDI libraries
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ring_buffer_lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pico-w-ble-midi-lib)

# Add executable
add_executable(mitimidi-relay
    main.c
)

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(mitimidi-relay 
    pico_stdlib
    pico_unique_id
    hardware_uart
    hardware_gpio
    hardware_i2c
    hardware_spi
    hardware_adc
    hardware_pwm
    hardware_clocks
    hardware_watchdog
    tinyusb_device
    tinyusb_board
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_cyw43
    ble_midi_server_lib
    ring_buffer_lib
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(mitimidi-relay 0)
pico_enable_stdio_uart(mitimidi-relay 1)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(mitimidi-relay)

# Add include directories
target_include_directories(mitimidi-relay PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Configure USB and Bluetooth
target_compile_definitions(mitimidi-relay PRIVATE
    CFG_TUSB_CONFIG_FILE="tusb_config.h"
    PICO_BTSTACK_CONFIG_FILE="btstack_config.h"
)

# Generate GATT header from .gatt file
pico_btstack_make_gatt_header(mitimidi-relay PRIVATE "${CMAKE_CURRENT_LIST_DIR}/midimiti.gatt" "${CMAKE_CURRENT_LIST_DIR}/pico-w-ble-midi-lib")
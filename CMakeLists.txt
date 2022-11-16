cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(picotermostato_project)

pico_sdk_init()

add_subdirectory(pico-onewire)

add_executable(picotermostato
    picotermostato.cpp
    display.cpp
    encoder.cpp
    sensor.cpp
    eeprom.cpp
)

target_link_libraries(picotermostato PRIVATE
    pico_stdlib
    pico_multicore
    pico_one_wire
    hardware_pio
    hardware_spi
    hardware_i2c
    hardware_dma
)

pico_generate_pio_header(picotermostato ${CMAKE_CURRENT_LIST_DIR}/encoder.pio)

pico_enable_stdio_usb(picotermostato 0)
pico_enable_stdio_uart(picotermostato 1)

pico_add_extra_outputs(picotermostato)


cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)

project(pico-mc-display-bridge-host C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_pico_midi_lib lib/pico_pico_midi_lib)

add_executable(pico-mc-display-bridge-host
    pico-mc-display-bridge-host.cpp
)

pico_enable_stdio_uart(pico-mc-display-bridge-host 1)

target_include_directories(pico-mc-display-bridge-host PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pico-mc-display-bridge-host pico_pico_midi_lib tinyusb_host tinyusb_board ring_buffer_lib pico_stdlib)

pico_add_extra_outputs(pico-mc-display-bridge-host)


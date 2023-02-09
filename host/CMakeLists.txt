cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)
set(target_name pico-mc-display-bridge-host)
project(${target_name} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_pico_midi_lib lib/pico_pico_midi_lib)

add_executable(pico-mc-display-bridge-host
    pico-mc-display-bridge-host.cpp
    nav_buttons.cpp
)

pico_enable_stdio_uart(pico-mc-display-bridge-host 1)

target_link_options(${target_name} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_name} PRIVATE -Wall -Wextra -DCFG_TUSB_DEBUG=1)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${target_name} pico_pico_midi_lib tinyusb_host tinyusb_board ring_buffer_lib pico_stdlib)

pico_add_extra_outputs(${target_name})


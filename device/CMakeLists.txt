cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)
set(target_name pico-mc-display-bridge-dev)
project(${target_name} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_USE_MALLOC_MUTEX=1")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_pico_midi_lib lib/pico_pico_midi_lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico-ssd1306-mono-graphics-lib lib/pico-ssd1306-mono-graphics-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico-mono-ui-lib lib/pico-mono-ui-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/midi-processor-lib lib/midi-processor-lib)

add_executable(${target_name}
    mc_bridge_usb_dev.cpp
    mc_channel_strip_display.cpp
    mc_channel_text.cpp
    mc_fader_sync.cpp
    mc_meter.cpp
    mc_seven_seg_display.cpp
    mc_vpot_display.cpp
    midi_processor_mc_display.cpp
    usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/../ext_lib/parson/parson.c
)

pico_enable_stdio_uart(${target_name} 1)

target_link_options(${target_name} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_name} PRIVATE -Wall -Wextra -DCFG_TUSB_DEBUG=1)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../ext_lib/parson)

target_link_libraries(${target_name} pico_pico_midi_lib tinyusb_device tinyusb_board ring_buffer_lib pico_stdlib
    ssd1306 ssd1306i2c ssd1306pioi2c text_box mono_graphics_lib littlefs-lib ui_menu ui_view_manager midi_processor_lib)

pico_add_extra_outputs(${target_name})


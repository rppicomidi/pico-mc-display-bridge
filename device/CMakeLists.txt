cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)

project(pico-mc-display-bridge-dev C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_USE_MALLOC_MUTEX=1")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../ext_lib/littlefs-lib ext_lib/littlefs-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_pico_midi_lib lib/pico_pico_midi_lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_mono_graphics_lib lib/pico_mono_graphics_lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_mono_ui_lib lib/pico_mono_ui_lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/core1_gpio_lib core1_gpio_lib)

add_executable(pico-mc-display-bridge-dev
    mc_bridge_model.cpp
    mc_bridge_usb_dev.cpp
    mc_channel_strip_display.cpp
    mc_channel_text.cpp
    mc_fader_sync.cpp
    mc_meter.cpp
    mc_settings_file.cpp
    mc_seven_seg_display.cpp
    mc_vpot_display.cpp
    setup_menu.cpp
    usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/../ext_lib/parson/parson.c
)

pico_enable_stdio_uart(pico-mc-display-bridge-dev 1)

target_include_directories(pico-mc-display-bridge-dev PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../ext_lib/parson)

target_link_libraries(pico-mc-display-bridge-dev pico_pico_midi_lib tinyusb_device tinyusb_board ring_buffer_lib pico_stdlib
    ssd1306 ssd1306i2c ssd1306pioi2c text_box mono_graphics_lib littlefs-lib core1_gpio_lib ui_menu ui_view_manager)

pico_add_extra_outputs(pico-mc-display-bridge-dev)


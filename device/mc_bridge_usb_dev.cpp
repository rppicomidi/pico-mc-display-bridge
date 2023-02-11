/**
 * @file mc_bridge_usb_dev.c
 * @brief This file contains the code that runs on the USB device
 * side of the pico-mc-bridge.
 *
 * MIT License
 * Copyright (c) 2022 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "mono_graphics_lib.h"
#include "ssd1306i2c.h"
#include "ssd1306pioi2c.h"
#include "ssd1306.h"
#include "mc_channel_strip_display.h"
#include "mc_seven_seg_display.h"
#include "pico/binary_info.h"
#include "pico_pico_midi_lib_config.h"
#include "pico_pico_midi_lib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "class/midi/midi_device.h"
#include "usb_descriptors.h"
#include "../common/pico-mc-display-bridge-cmds.h"
#include "settings_file.h"
#include "view_manager.h"
#include "midi_processor_manager.h"
#include "midi_processor_home_screen.h"
#include "midi_processor_mc_display.h"
#include "midi_processor_no_settings_view.h"
// On-board LED mapping. If no LED, set to NO_LED_GPIO
const uint NO_LED_GPIO = 255;
const uint LED_GPIO = 25;
// UART selection Pin mapping. You can move these for your design if you want to
// Make sure all these values are consistent with your choice of midi_uart
#define MIDI_UART_NUM 1
const uint MIDI_UART_TX_GPIO = 4;
const uint MIDI_UART_RX_GPIO = 5;

static uint16_t render_done_mask = 0;
static void callback(uint8_t display_num)
{
    render_done_mask |= (1u << display_num);
}

namespace rppicomidi {
class Pico_mc_display_bridge_dev {
public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Pico_mc_display_bridge_dev& instance()
    {
        static Pico_mc_display_bridge_dev _instance; // Guaranteed to be destroyed.
                                             // Instantiated on first use.
        return _instance;
    }
    Pico_mc_display_bridge_dev(Pico_pico_midi_lib const&) = delete;
    void operator=(Pico_mc_display_bridge_dev const&) = delete;

    Pico_mc_display_bridge_dev();
    void task();
    uint8_t serial_number[7];
    static struct Rx_packet {
        uint8_t packet[4];
        uint8_t idx;
        uint8_t size;
    } rx_packet[16]; // rx_packet[cbl] is the currently decoded packet for cable number cbl 0-15
    static void midi_cb(uint8_t *buffer, uint8_t buflen, uint8_t cable_num);
    static void static_cmd_cb(uint8_t header, uint8_t* buffer, uint16_t length);
    uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid);
    void get_product_string(uint8_t idx, char* strbuf);
    void cmd_cb(uint8_t header, uint8_t* buffer, uint16_t length);
    void poll_usb_rx(bool connected);
    void poll_midi_uart_rx(bool connected);
    bool handle_mc_device_inquiry();
    void push_to_midi_uart(uint8_t* bytes, int nbytes, uint8_t cable_num);
    void request_dev_desc();
    const uint NO_LED_GPIO=255;
    const uint LED_GPIO=25;
    const uint8_t OLED_ADDR=0x3c;
    uint8_t addr[1];
    const uint8_t MUX_ADDR=0;
    uint8_t* mux_map=nullptr;
    static const uint8_t num_chan_displays = 8;
    const uint8_t OLED_CH_1_SDA_GPIO = 6;
    const uint8_t OLED_CH_1_SCL_GPIO = 7;
    const uint8_t OLED_CH_2_SDA_GPIO = 8;
    const uint8_t OLED_CH_2_SCL_GPIO = 9;
    const uint8_t OLED_CH_3_SDA_GPIO = 10;
    const uint8_t OLED_CH_3_SCL_GPIO = 11;
    const uint8_t OLED_CH_4_SDA_GPIO = 12;
    const uint8_t OLED_CH_4_SCL_GPIO = 13;
    const uint8_t OLED_CH_5_SDA_GPIO = 14;
    const uint8_t OLED_CH_5_SCL_GPIO = 15;
    const uint8_t OLED_CH_6_SDA_GPIO = 16;
    const uint8_t OLED_CH_6_SCL_GPIO = 17;
    const uint8_t OLED_CH_7_SDA_GPIO = 18;
    const uint8_t OLED_CH_7_SCL_GPIO = 19;
    const uint8_t OLED_CH_8_SDA_GPIO = 20;
    const uint8_t OLED_CH_8_SCL_GPIO = 21;
    const uint8_t OLED_TC_SDA_GPIO = 2;
    const uint8_t OLED_TC_SCL_GPIO = 3;
    Ssd1306pio_i2c i2c_driver0{pio0, 0, 0, addr, OLED_CH_1_SDA_GPIO, OLED_CH_1_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver1{pio0, 1, i2c_driver0.get_offset(), addr, OLED_CH_2_SDA_GPIO, OLED_CH_2_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver2{pio0, 2, i2c_driver0.get_offset(), addr, OLED_CH_3_SDA_GPIO, OLED_CH_3_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver3{pio0, 3, i2c_driver0.get_offset(), addr, OLED_CH_4_SDA_GPIO, OLED_CH_4_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver4{pio1, 0, 0, addr, OLED_CH_5_SDA_GPIO, OLED_CH_5_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver5{pio1, 1, i2c_driver4.get_offset(), addr, OLED_CH_6_SDA_GPIO, OLED_CH_6_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver6{pio1, 2, i2c_driver4.get_offset(), addr, OLED_CH_7_SDA_GPIO, OLED_CH_7_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306pio_i2c i2c_driver7{pio1, 3, i2c_driver4.get_offset(), addr, OLED_CH_8_SDA_GPIO, OLED_CH_8_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};
    Ssd1306i2c i2c_driver_tc{i2c1, addr, OLED_TC_SDA_GPIO, OLED_TC_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};

    Ssd1306 ssd1306_0;
    Ssd1306 ssd1306_1;
    Ssd1306 ssd1306_2;
    Ssd1306 ssd1306_3;
    Ssd1306 ssd1306_4;
    Ssd1306 ssd1306_5;
    Ssd1306 ssd1306_6;
    Ssd1306 ssd1306_7;
    Ssd1306 ssd1306_tc;
    Mono_graphics screen0;
    Mono_graphics screen1;
    Mono_graphics screen2;
    Mono_graphics screen3;
    Mono_graphics screen4;
    Mono_graphics screen5;
    Mono_graphics screen6;
    Mono_graphics screen7;
    Mono_graphics* screen[num_chan_displays];
    Mono_graphics screen_tc;
    Mc_channel_strip_display channel_disp0;
    Mc_channel_strip_display channel_disp1;
    Mc_channel_strip_display channel_disp2;
    Mc_channel_strip_display channel_disp3;
    Mc_channel_strip_display channel_disp4;
    Mc_channel_strip_display channel_disp5;
    Mc_channel_strip_display channel_disp6;
    Mc_channel_strip_display channel_disp7;
    Mc_channel_strip_display* channel_disp[num_chan_displays];
    View_manager tc_view_manager;
    Home_screen setup_menu;
    Mc_seven_seg_display seven_seg;
    void *midi_uart_instance;
    enum {
        Dev_descriptor, // Getting the Device Descriptor
        Conf_descriptor, // Getting the configuration descriptor
        Langids, // Getting the Language ID list (string descriptor 0)
        String_list, // Getting the string index list
        All_strings, // Getting every string for every supported language 
        Operating    // Normal MIDI I/O operation
    } state;
    std::vector<uint8_t> istrings;
    size_t istrings_idx;
    std::vector<uint16_t> langids;
    size_t langids_idx;
    struct Usb_string_s {
        Usb_string_s() : index{0}, length{0}, langid{0}, utf16le{nullptr} {}
        uint8_t index;  // the string index from the USB descriptor
        uint8_t length; // number of 16-bit utf16le character codes in the string
        uint16_t langid; // the language ID associated with this string.
        uint16_t* utf16le; // points to memory allocated to hold the string.
    };
    std::vector<Usb_string_s> string_list;
};
}

rppicomidi::Pico_mc_display_bridge_dev::Rx_packet rppicomidi::Pico_mc_display_bridge_dev::rx_packet[16];

static void blink_led(void)
{
    static absolute_time_t previous_timestamp = {0};

    static bool led_state = false;

    // This design has no on-board LED
    if (NO_LED_GPIO == LED_GPIO)
        return;
    absolute_time_t now = get_absolute_time();
    
    int64_t diff = absolute_time_diff_us(previous_timestamp, now);
    if (diff > 1000000) {
        gpio_put(LED_GPIO, led_state);
        led_state = !led_state;
        previous_timestamp = now;
    }
}

rppicomidi::Pico_mc_display_bridge_dev::Pico_mc_display_bridge_dev()  : addr{OLED_ADDR},
    ssd1306_0{&i2c_driver0, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_1{&i2c_driver1, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_2{&i2c_driver2, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_3{&i2c_driver3, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_4{&i2c_driver4, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_5{&i2c_driver5, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_6{&i2c_driver6, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_7{&i2c_driver7, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    ssd1306_tc{&i2c_driver_tc, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0},
    screen0{&ssd1306_0, Display_rotation::Portrait270},
    screen1{&ssd1306_1, Display_rotation::Portrait270},
    screen2{&ssd1306_2, Display_rotation::Portrait270},
    screen3{&ssd1306_3, Display_rotation::Portrait270},
    screen4{&ssd1306_4, Display_rotation::Portrait270},
    screen5{&ssd1306_5, Display_rotation::Portrait270},
    screen6{&ssd1306_6, Display_rotation::Portrait270},
    screen7{&ssd1306_7, Display_rotation::Portrait270},
    screen{&screen0, &screen1, &screen2, &screen3, &screen4, &screen5, &screen6, &screen7},
    screen_tc{&ssd1306_tc, Display_rotation::Landscape0},
    channel_disp0{screen0, 0},
    channel_disp1{screen1, 1},
    channel_disp2{screen2, 2},
    channel_disp3{screen3, 3},
    channel_disp4{screen4, 4},
    channel_disp5{screen5, 5},
    channel_disp6{screen6, 6},
    channel_disp7{screen7, 7},
    channel_disp{&channel_disp0,&channel_disp1,&channel_disp2,&channel_disp3,&channel_disp4,&channel_disp5,&channel_disp6,&channel_disp7},
    setup_menu{screen_tc, ""},
    seven_seg{tc_view_manager, screen_tc, false, false, setup_menu},
    state{Dev_descriptor}
{
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    Pico_pico_midi_lib::instance().init(nullptr, static_cmd_cb);
    memset(rx_packet, 0, sizeof(rx_packet));
    render_done_mask = 0;

    uint16_t target_done_mask = ((1<<(num_chan_displays)) -1) |(1<<8);
    bool success = true;
    for (size_t idx=0; success && idx < num_chan_displays; idx++) {
        screen[idx]->render_non_blocking(callback, idx);
    }
    tc_view_manager.push_view(&seven_seg);
    screen_tc.render_non_blocking(callback, 8);
    while (success && render_done_mask != target_done_mask) {
        for (size_t idx=0; success && idx < num_chan_displays; idx++) {
            success = screen[idx]->task();
        }
        if (success)
            success = screen_tc.task();
    }
    assert(success);
    Midi_processor_mc_display_core::instance().init(num_chan_displays, channel_disp, &seven_seg);
    // create the instance of the MIDI Processor Manager attach the screen
    Midi_processor_manager::instance().set_screen(&screen_tc);
    Midi_processor_manager::instance().add_new_processor_type(Midi_processor_mc_display::static_getname(), Midi_processor_mc_display::static_make_new,
                                                              Midi_processor_no_settings_view::static_make_new);

    // start the reporting of the settings encoder sw and settings encoder positions
    //assert(core1_gpio.request_status_update());
}

void rppicomidi::Pico_mc_display_bridge_dev::static_cmd_cb(uint8_t header, uint8_t* payload_, uint16_t length_)
{
    instance().cmd_cb(header, payload_, length_);
}

void rppicomidi::Pico_mc_display_bridge_dev::cmd_cb(uint8_t header, uint8_t* payload_, uint16_t length_)
{
    switch (header) {
        case RETURN_DEV_DESC:
            if (state == Dev_descriptor) {
                assert(length_ == 18);
                set_dev_desc(payload_);
                state = Conf_descriptor;
                rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_CONF_DESC_0, nullptr, 0);
            }
            break;
        case RETURN_CONF_DESC_0:
            if (state == Conf_descriptor && length_ > 0) {
                assert(create_midi_configuration_descriptors(payload_));
                if (get_midi_configuration_length() > CONFIG_DESC_MAX_PAYLOAD) {
                    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_CONF_DESC_1, nullptr, 0);
                }
                else {
                    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_LANGIDS, nullptr, 0);
                    state = Langids;
                }
            }
            break;
        case RETURN_CONF_DESC_1:
            if (state == Conf_descriptor && length_ > 0) {
                assert(append_midi_configuration_descriptors(payload_, CONFIG_DESC_MAX_PAYLOAD, length_));
                if (get_midi_configuration_length() > (CONFIG_DESC_MAX_PAYLOAD*2)) {
                    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_CONF_DESC_2, nullptr, 0);
                }
                else {
                    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_LANGIDS, nullptr, 0);
                    state = Langids;
                }
            }
            break;
        case RETURN_CONF_DESC_2:
            if (state == Conf_descriptor && length_ > 0) {
                assert(append_midi_configuration_descriptors(payload_, CONFIG_DESC_MAX_PAYLOAD*2, length_));
                state = Langids;
                rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_LANGIDS, nullptr, 0);
            }
            break;
        case RETURN_DEV_LANGIDS:
            if (state == Langids && length_ > 0) {
                uint8_t num_langids = length_ / 2;
                uint16_t* langid_ptr = (uint16_t*)payload_;
                for (int idx=0; idx < num_langids; idx++) {
                    langids.push_back(*langid_ptr++);
                }

                state = String_list;
                rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_STRING_IDXS, nullptr, 0);
            }
            break;
        case RETURN_DEV_STRING_IDXS:
            if (instance().state == String_list) {
                for (uint8_t idx = 0; idx < length_; idx++) {
                    instance().istrings.push_back(payload_[idx]);
                }
                // start getting all the strings
                state = All_strings;
                istrings_idx = 0;
                langids_idx = 0;
                uint8_t payload[3];
                payload[0] = istrings[instance().istrings_idx];
                payload[1] = langids[instance().langids_idx] & 0xff;
                payload[2] = (langids[instance().langids_idx] >> 8) & 0xff;
                rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_STRING, payload, 3);
            }
            break;
        case RETURN_DEV_STRING:
            if (state == All_strings) {
                rppicomidi::Pico_mc_display_bridge_dev::Usb_string_s usb_string;
                usb_string.index = payload_[0];
                usb_string.langid = (uint16_t)payload_[1] | ((uint16_t)payload_[2]<< 8);
                usb_string.length = (length_-3)/2;
                usb_string.utf16le = new uint16_t[usb_string.length];
                memcpy((uint8_t*)usb_string.utf16le, payload_+3, length_-3);
                string_list.push_back(usb_string);
                istrings_idx++;
                if (istrings_idx >= istrings.size()) {
                    istrings_idx = 0;
                    if (++langids_idx >= langids.size()) {
                        // Got all the strings
                        // The whole configuration descriptor is read in at this point.
                        // Can recall settings for the connected device
                        uint16_t vid, pid;
                        uint8_t num_rx_cables, num_tx_cables;
                        get_vid_pid(&vid,&pid);
                        get_num_cables(&num_rx_cables, &num_tx_cables);
                        Settings_file::instance().set_vid_pid(vid,pid);
                        char devstr[129];
                        get_product_string(get_product_string_index(), devstr);
                        Midi_processor_manager::instance().set_connected_device(vid, pid, devstr, num_rx_cables, num_tx_cables);
                        setup_menu.set_connected_device(devstr,num_rx_cables, num_tx_cables, false);
                        if (!Settings_file::instance().load()) {
                            Midi_processor_manager::instance().clear_all_processors();
                        }
                        // Now ready to start running
                        state = Operating;
                        rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(RESYNCHRONIZE, nullptr, 0);
                    }
                }
                if (state == All_strings) {
                    // still more strings to get. Get the next one
                    uint8_t payload[3];
                    payload[0] = istrings[istrings_idx];
                    payload[1] = langids[langids_idx] & 0xff;
                    payload[2] = (langids[langids_idx] >> 8) & 0xff;
                    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_STRING, payload, 3);
                }
            }
            break;
        case RETURN_NAV_BUTTON_STATE:
            if (length_ == 1) {
                bool is_shifted = payload_[0] & NAV_BUTTON_SHIFT;
                switch(payload_[0] & NAV_BUTTON_DIR_MASK) {
                    default:
                        if (payload_[0] & NAV_BUTTON_SELECT) {
                            tc_view_manager.on_select();
                        }
                        else if (payload_[0] & NAV_BUTTON_BACK) {
                            if (is_shifted)
                                tc_view_manager.go_home();
                            else
                                tc_view_manager.on_back();
                        }
                        break;
                    case NAV_BUTTON_UP:
                        tc_view_manager.on_increment(1, is_shifted);
                        break;
                    case NAV_BUTTON_DOWN:
                        tc_view_manager.on_decrement(1, is_shifted);
                        break;
                    case NAV_BUTTON_LEFT:
                        tc_view_manager.on_left(1, is_shifted);
                        break;
                    case NAV_BUTTON_RIGHT:
                        tc_view_manager.on_right(1, is_shifted);
                        break;
                }
            }
            break;
    }
}

void rppicomidi::Pico_mc_display_bridge_dev::request_dev_desc()
{
    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_DESC, nullptr, 0);
}

void rppicomidi::Pico_mc_display_bridge_dev::midi_cb(uint8_t *rx, uint8_t buflen, uint8_t cable_num)
{
    for (uint8_t idx=0; idx<buflen; idx++) {
        if (rx_packet[cable_num].idx == 0) {
            // starting a new packet
            uint8_t const msg = ((*rx) >> 4) & 0xF;
            bool sysex_in_progress = (msg < MIDI_CIN_NOTE_OFF) && (rx_packet[cable_num].packet[0] & 0xF) == MIDI_CIN_SYSEX_START;
            rx_packet[cable_num].packet[0] = cable_num << 4;
            rx_packet[cable_num].packet[1] = *rx++;
            rx_packet[cable_num].idx = 2;
            if (msg >= MIDI_CIN_NOTE_OFF && msg <= MIDI_CIN_PITCH_BEND_CHANGE) {
                // channel message
                rx_packet[cable_num].packet[0] |= msg;
                if (msg == MIDI_CIN_CHANNEL_PRESSURE || msg == MIDI_CIN_PROGRAM_CHANGE) {
                    rx_packet[cable_num].size = 3;
                    rx_packet[cable_num].packet[3] = 0;
                }
                else {
                    rx_packet[cable_num].size = 4;
                }
            }
            else if (rx_packet[cable_num].packet[1] == 0xF0 || sysex_in_progress) {
                // cannot determine the proper size, but assume it needs at least 2 data bytes before EOX
                // adjust if EOX comes sooner
                rx_packet[cable_num].size = 4; 
                rx_packet[cable_num].packet[0] |= MIDI_CIN_SYSEX_START;
            }
            else if (rx_packet[cable_num].packet[1] >= 0xF4) {
                rx_packet[cable_num].packet[0] |= MIDI_CIN_SYSEX_END_1BYTE;
                rx_packet[cable_num].size = 2;
                rx_packet[cable_num].idx = 0; // we have the whole packet
                rx_packet[cable_num].packet[2] = 0;
                rx_packet[cable_num].packet[3] = 0;
            }
            else if (rx_packet[cable_num].packet[1] == 0xF1 || rx_packet[cable_num].packet[1] == 0xF3) {
                rx_packet[cable_num].packet[0] |= MIDI_CIN_SYSCOM_2BYTE;
                rx_packet[cable_num].size = 3; // 2 byte system message
                rx_packet[cable_num].packet[3] = 0;
            }
            else if (rx_packet[cable_num].packet[1] == 0xF2) {
                rx_packet[cable_num].packet[0] |= MIDI_CIN_SYSCOM_3BYTE;
                rx_packet[cable_num].size = 4; // 3 byte system message
            }
            else {
                // bad MIDI data stream
                rx_packet[cable_num].idx = 0;
                rx_packet[cable_num].size = 0;
                printf("dropped MIDI stream byte idx=0 cable %u data 0x%02x\r\n", cable_num, rx_packet[cable_num].packet[1]);
            }
        }
        else if (*rx == 0xF7) {
            // EOX
            uint8_t idx = rx_packet[cable_num].idx;
            if (idx == 2) {
                rx_packet[cable_num].packet[0] = (cable_num << 4) | MIDI_CIN_SYSEX_END_2BYTE;
                rx_packet[cable_num].packet[3] = 0;
                rx_packet[cable_num].size = 3;
            }
            else {
                rx_packet[cable_num].packet[0] = (cable_num << 4) | MIDI_CIN_SYSEX_END_3BYTE;
            }
            rx_packet[cable_num].packet[idx] = *rx++;
            rx_packet[cable_num].idx = 0; // we have the whole packet
        }
        else if (rx_packet[cable_num].idx < rx_packet[cable_num].size) {
            // filling the packet
            rx_packet[cable_num].packet[rx_packet[cable_num].idx++] = *rx++;
            if (rx_packet[cable_num].idx == rx_packet[cable_num].size) {
                rx_packet[cable_num].idx = 0;
            }
        }
        else {
            // packet overflow
            printf("packet overflow\r\n");
        }
        if (rx_packet[cable_num].idx == 0 && rx_packet[cable_num].size != 0) {
            // received a whole packet; process it
            if (Midi_processor_manager::instance().filter_midi_in(cable_num, rx_packet[cable_num].packet)) {
                tud_midi_packet_write(rx_packet[cable_num].packet);
                //tud_midi_stream_write(cable_num, rx_packet[cable_num].packet, 4);
            }
        }
    }
}

void rppicomidi::Pico_mc_display_bridge_dev::push_to_midi_uart(uint8_t* bytes, int nbytes, uint8_t cable_num)
{
    uint8_t npushed = Pico_pico_midi_lib::instance().write_midi_to_tx_buffer(bytes,nbytes,cable_num);
    if (npushed != nbytes) {
        TU_LOG1("Warning: Dropped %d bytes sending to UART MIDI Out\r\n", nbytes - npushed);
    }
}

#if 0
bool rppicomidi::Pico_mc_display_bridge_dev::handle_mc_device_inquiry()
{
    int nread = sysex_idx+1;
    bool handled = false;
    if (nread==7 && sysex_message[1]==0x00 && sysex_message[2]==0x00 && sysex_message[3]==0x66 &&
        sysex_message[4]==0x14 && sysex_message[5]==0x00 && sysex_message[6] == 0xf7) {
        // MC device query message; respond with the MC Host Connection Query
        uint8_t host_connection_query[]= { 0xF0, // start sysex
            0x00, 0x00, 0x66, // Mackie Manufacturer ID
            0x14, // MCU Pro (0x14) with all displays
            0x01, // Host Connection Query message ID
            serial_number[0], serial_number[1], serial_number[2], serial_number[3], serial_number[4], serial_number[5], serial_number[6], // arbitrary serial number
            0x01, 0x02, 0x03, 0x04, // arbitray
            0xf7, // EOX
        };
        uint32_t nwritten = tud_midi_stream_write(0, host_connection_query, sizeof(host_connection_query));
        if (nwritten != sizeof(host_connection_query)) {
            TU_LOG1("Warning: Dropped %lu bytes of host_connection_query message\r\n", sizeof(host_connection_query) - nwritten);
        }
        else {
            TU_LOG1("Sent Host Connection Query\r\n");
        }
        handled = true;
    }
    else if (nread==8 && sysex_message[1]==0x00 && sysex_message[2]==0x00 && sysex_message[3]==0x66 &&
        sysex_message[4]==0x14 && sysex_message[5]==0x1A && sysex_message[6] == 0x00 && sysex_message[7] == 0xf7) {
        uint8_t serial_number_response[] = { 0xF0, // start sysex
            0x00, 0x00, 0x66, // Mackie Manufacturer ID
            0x14, // MCU Pro
            0x1B, 0x58, 0x59, 0x5A, // Response to serial number request
            serial_number[0], serial_number[1], serial_number[2], serial_number[3], // The serial number
            0xF7 // EOX
        };
        uint32_t nwritten = tud_midi_stream_write(0, serial_number_response, sizeof(serial_number_response));
        if (nwritten != sizeof(serial_number_response)) {
            TU_LOG1("Warning: Dropped %lu bytes of serial_number_response message\r\n", sizeof(serial_number_response) - nwritten);
        }
        else {
            TU_LOG1("Sent Serial Number Response\r\n");
        }
        handled = true;
    }
    else if (nread >= 6 && sysex_message[1]==0x00 && sysex_message[2]==0x00 && sysex_message[3]==0x66 && sysex_message[4]==0x15) {
        // message is for an extender. Ignore it
        handled = true;
    }
    return handled;
}
#endif
#ifdef LOG_MIDI_TO_RAM
volatile static uint8_t midi_log[1024*16];
static int midi_log_idx = 0;
#endif

void rppicomidi::Pico_mc_display_bridge_dev::poll_usb_rx(bool connected)
{
    if (!connected || !tud_midi_available())
    {
        return;
    }
    uint8_t rx[4];
    //uint8_t nread = 0;

    while(tud_midi_n_packet_read(0, rx)) {
        uint8_t const cable_num = (rx[0] >> 4) & 0xf;
        if (!Midi_processor_manager::instance().filter_midi_out(cable_num, rx)) {
            continue; // packet filtered out.
        }
#if 0
        uint8_t const code_index = rx[0] & 0x0f;
        // MIDI 1.0 Table 4-1: Code Index Number Classifications
        switch(code_index)
        {
            case MIDI_CIN_MISC:
            case MIDI_CIN_CABLE_EVENT:
                // These are reserved and unused, possibly issue somewhere, skip this packet
                continue;
                break;

            case MIDI_CIN_SYSEX_END_1BYTE:
            case MIDI_CIN_1BYTE_DATA:
                nread = 1;
                break;

            case MIDI_CIN_SYSCOM_2BYTE     :
            case MIDI_CIN_SYSEX_END_2BYTE  :
            case MIDI_CIN_PROGRAM_CHANGE   :
            case MIDI_CIN_CHANNEL_PRESSURE :
                nread = 2;
                break;

            default:
                nread = 3;
                break;
        }
        bool success = false;
        int jdx;
        int dropped_sysex = 0;
        switch(rx[1]) {
            case 0x90: // Note on (LED control message)
                // Note message, channel 1
                success = seven_seg.set_smpte_beats_by_mc_note(rx[2], rx[3]);
                if (!success) {
                    for (int chan = 0; !success && chan < num_chan_displays; chan++) {
                        success = channel_disp[chan]->push_midi_message(rx+1, nread);
                    }
                }
                // Pass this on
                push_to_midi_uart(rx+1, nread, cable_num);
                break;
            case 0xB0: // Logic Control 7-segment message or VPot message
                // CC message
                success = seven_seg.set_digit_by_mc_cc(rx[2], rx[3]);
                // filter this out if is a 7-seg LED digit message
                if (!success) {
                    push_to_midi_uart(rx+1, nread,cable_num);
                }
                if (!success) {
                    for (int chan = 0; !success && chan < num_chan_displays; chan++) {
                        success = channel_disp[chan]->push_midi_message(rx +1, 3);
                    }
                }
                // only push the message if not a display message
                if (!success)
                    push_to_midi_uart(rx+1, nread, cable_num);
                break;
            case 0xBF: // 7-segment display message alternate
                // Alternate CC message for timecode.
                success = seven_seg.set_digit_by_mc_cc(rx[2], rx[3]);
                // only push the message if not a display message
                if (!success)
                    push_to_midi_uart(rx+1, nread, cable_num);
                // filter this out
                break;
            case 0xD0:  // Channel pressure (meter message)
                for (int chan = 0; !success && chan < num_chan_displays; chan++) {
                    success = channel_disp[chan]->push_midi_message(rx +1, 2);
                }
                // only push the message if not a display message
                if (!success)
                    push_to_midi_uart(rx+1, nread, cable_num);
                break;
            case 0xF0: // sysex start
                sysex_message[0] = rx[1];
                sysex_idx = 1;
                dropped_sysex = 0;
                for (jdx = 1; jdx <= nread && rx[jdx] != 0xF7; jdx++) {
                    sysex_message[sysex_idx] = rx[jdx];
                    // truncate sysex messages longer than we can store but leave room for EOX
                    if (sysex_idx < max_sysex-1) {
                        ++sysex_idx;
                    }
                    else {
                        ++dropped_sysex;
                    }
                }
                if (jdx == nread) {
                    waiting_for_eox = true;
                }
                else {
                    // must have found eox
                    sysex_message[sysex_idx] = rx[jdx]; // copy eox
                    if (dropped_sysex != 0) {
                        TU_LOG1("Warning dropped %d sysex bytes\r\n", dropped_sysex);
                    }
                    if (!handle_mc_device_inquiry()) {
                        if (!seven_seg.set_digits_by_mc_sysex(sysex_message, sysex_idx+1)) {
                            int nsuccessful = 0;
                            // have to try every channel strip
                            for (int chan = 0; chan < num_chan_displays; chan++) {
                                if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                    ++nsuccessful;
                                }
                            }
                            success = nsuccessful != 0;
                        }
                        if (!success) {
                            // send it on over the MIDI UART
                            push_to_midi_uart(sysex_message, sysex_idx+1,cable_num);

                            printf("unhandled:\r\n");
                            for (size_t kdx=0; kdx<=sysex_idx;kdx++) {
                                printf("%02x ", sysex_message[kdx]);
                            }
                            printf("\n\r");
                        }
                    }
                    waiting_for_eox = false;
                    sysex_idx = 0;
                }
                break;
            case 0xF7: // EOX
                if (waiting_for_eox) {
                    if (dropped_sysex != 0) {
                        TU_LOG1("Warning dropped %d sysex bytes\r\n", dropped_sysex);
                    }

                    sysex_message[++sysex_idx] = rx[1];
                    waiting_for_eox = false;
                    if (!handle_mc_device_inquiry()) {
                        if (!seven_seg.set_digits_by_mc_sysex(sysex_message, sysex_idx+1)) {
                            // have to try every channel strip
                            int nsuccessful = 0;
                            for (int chan = 0; chan < num_chan_displays; chan++) {
                                if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                    ++nsuccessful;
                                }
                            }
                            success = nsuccessful != 0;
                        }
                        if (!success) {
                            push_to_midi_uart(sysex_message, sysex_idx+1, cable_num); // send it on to the MIDI UART
                            printf("unhandled:\r\n");
                            for (size_t kdx=0; kdx<=sysex_idx;kdx++) {
                                printf("%02x ",sysex_message[kdx]);
                            }
                            printf("\n\r");
                        }
                    }
                }
                waiting_for_eox = false;
                sysex_idx = 0;
                break;
            default:
                if (waiting_for_eox) {
                    // Might be the middle of a sysex message or the tail end of one
                    for (jdx = 1; jdx <=nread && (rx[jdx] & 0x80) == 0; jdx++) {
                        sysex_message[sysex_idx++] = rx[jdx];
                    }
                    if ((rx[jdx] & 0x80) == 0) {
                        if (rx[jdx] == 0xF7) {
                            // we have the sysex whole message message
                            if (dropped_sysex != 0) {
                                TU_LOG1("Warning dropped %d sysex bytes\r\n", dropped_sysex);
                            }

                            sysex_message[++sysex_idx] = rx[1];
                            waiting_for_eox = false;
                            if (!handle_mc_device_inquiry()) {
                                if (!seven_seg.set_digits_by_mc_sysex(sysex_message, sysex_idx+1)) {
                                    // have to try every channel strip
                                    int nsuccessful = 0;
                                    for (int chan = 0; chan < num_chan_displays; chan++) {
                                        if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                            ++nsuccessful;
                                        }
                                    }
                                    success = nsuccessful != 0;
                                }
                                if (!success) {
                                    push_to_midi_uart(sysex_message, sysex_idx+1, cable_num); // send it on to the MIDI UART
                                    printf("unhandled:\r\n");
                                    for (size_t kdx=0; kdx<=sysex_idx;kdx++) {
                                        printf("%02x ",sysex_message[kdx]);
                                    }
                                    printf("\n\r");
                                }
                            }
                        }
                        else
                        {
                            // Something is very wrong. USB doesn't support inserting messages mid-stream
                        }
                        waiting_for_eox = false;
                        sysex_idx = 0;
                    }
                }
                else {
                    // Just an unhandled message. Pass it on for the host
                    push_to_midi_uart(rx+1, nread, cable_num);
                }
                break;
        }
        success = false;
        #endif
    } // end processing one USB MIDI packet
}


void rppicomidi::Pico_mc_display_bridge_dev::task()
{
    bool connected = tud_midi_mounted();
    // poll MIDI receive from UART and USB
    poll_midi_uart_rx(connected);
    poll_usb_rx(connected);
    // Drain any transmissions that result
    Pico_pico_midi_lib::instance().drain_tx_buffer();

    // Update the screens if need be
    for (int chan = 0; chan < num_chan_displays; chan++) {
        channel_disp[chan]->task();
        if (screen[chan]->can_render()) {
            screen[chan]->render_non_blocking(nullptr, chan);
        }
        screen[chan]->task();
    }

    if (screen_tc.can_render()) {
        screen_tc.render_non_blocking(nullptr, 0);
    }
    screen_tc.task();
}
void rppicomidi::Pico_mc_display_bridge_dev::poll_midi_uart_rx(bool connected)
{
    if (connected) {
         Pico_pico_midi_lib::instance().poll_rx_buffer(); // midi_cb() or cmd_cb() will get called if there is data
    }
}

int main()
{
    using namespace rppicomidi;
    board_init();
    printf("Pico MC Bridge: USB Device\r\n");

    Pico_mc_display_bridge_dev::instance().request_dev_desc(); // initialize the instance and request the connected device info
    absolute_time_t then = get_absolute_time();
    while (Pico_mc_display_bridge_dev::instance().state != Pico_mc_display_bridge_dev::Operating) {
        Pico_mc_display_bridge_dev::instance().poll_midi_uart_rx(true);
        // Drain any transmissions that result
        Pico_pico_midi_lib::instance().drain_tx_buffer();
        absolute_time_t now = get_absolute_time();
        int64_t diff = absolute_time_diff_us(then, now);
        switch(Pico_mc_display_bridge_dev::instance().state) {
            case Pico_mc_display_bridge_dev::Dev_descriptor:
                if (diff > 1000000) {
                    Pico_mc_display_bridge_dev::instance().request_dev_desc(); // initialize the instance and request the connected device info
                    then = now;
                }
                break;
            default:
                // otherwise, just wait for the state to switch to Operating
                break;
        }
    }
    Pico_pico_midi_lib::instance().init(Pico_mc_display_bridge_dev::midi_cb, Pico_mc_display_bridge_dev::static_cmd_cb);
    tusb_init();
    while (1) {
        tud_task();

        blink_led();
        Pico_mc_display_bridge_dev::instance().task();
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // TODO
    TU_LOG1("Mounted\r\n");
    #if 0
    uint64_t now = time_us_64();
    using namespace rppicomidi;
    Pico_mc_display_bridge_dev::instance().serial_number[0] = now & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[1] = (now >> 8ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[2] = (now >> 16ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[3] = (now >> 24ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[4] = (now >> 32ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[5] = (now >> 40ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[6] = (now >> 48ull) & 0x7Full;
    #endif
    rppicomidi::Midi_processor_mc_display_core::instance().create_serial_number();
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    // TODO
    TU_LOG1("Unmounted\r\n");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    // TODO
    TU_LOG1("Suspended\r\n");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    // TODO
    TU_LOG1("Resumed\r\n");
}

uint16_t const* rppicomidi::Pico_mc_display_bridge_dev::tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static uint16_t str_desc[256];
    if (index == 0) {
        //langid list
        uint8_t length = langids.size()*2+2;
        str_desc[0] = length | (TUSB_DESC_STRING << 8 );
        uint16_t* ptr = str_desc+1;
        for (auto& langid:langids) {
            *ptr++ = langid;
        }
        return str_desc;
    }
    for (auto& str: string_list) {
        if (str.index == index && str.langid == langid) {
            assert(str_desc);
            str_desc[0] = (str.length+2) | (TUSB_DESC_STRING << 8 );
            memcpy((uint8_t*)(str_desc+1), (uint8_t*)str.utf16le, str.length);
            return str_desc;
        }
    }
    return NULL;
}

// handle string descriptor requests from the USB Host using
// strings from the Host Pico's connected device
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    return rppicomidi::Pico_mc_display_bridge_dev::instance().tud_descriptor_string_cb(index, langid);
}

void rppicomidi::Pico_mc_display_bridge_dev::get_product_string(uint8_t stridx, char* strbuf)
{
    if (stridx && strbuf != nullptr) {
        *strbuf='\0';
        for (auto str: string_list) {
            if (str.index == stridx) {
                for (uint8_t idx = 0; idx < str.length; idx++ ) {
                    *strbuf++ = str.utf16le[idx] & 0xff;
                }
                *strbuf = '\0';
            }
        }
    }
}
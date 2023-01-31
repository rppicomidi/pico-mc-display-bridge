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
#include "mc_fader_sync.h"
#include "pico/binary_info.h"
#include "pico_pico_midi_lib_config.h"
#include "pico_pico_midi_lib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "class/midi/midi_device.h"
#include "usb_descriptors.h"
#include "../common/pico-mc-display-bridge-cmds.h"
#include "mc_bridge_model.h"
#include "mc_settings_file.h"
#include "core1_gpio_lib.h"
#include "view_manager.h"
#include "setup_menu.h"

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

    static void midi_cb(uint8_t *buffer, uint8_t buflen, uint8_t cable_num);
    static void static_cmd_cb(uint8_t header, uint8_t* buffer, uint16_t length);
    uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid);
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
    Mono_graphics* screen[8];
    Mono_graphics screen_tc;
    Mc_channel_strip_display channel_disp0;
    Mc_channel_strip_display channel_disp1;
    Mc_channel_strip_display channel_disp2;
    Mc_channel_strip_display channel_disp3;
    Mc_channel_strip_display channel_disp4;
    Mc_channel_strip_display channel_disp5;
    Mc_channel_strip_display channel_disp6;
    Mc_channel_strip_display channel_disp7;
    Mc_channel_strip_display* channel_disp[8];
    View_manager tc_view_manager;
    Mc_bridge_model model;
    Mc_settings_file settings_file;
    Setup_menu setup_menu;
    Mc_seven_seg_display seven_seg;
    static const size_t max_sysex = 2048; // the maximum MC SySex message is 120 bytes long, but fortune favors the prepared

    // TODO group these two into an array of structs; one element for every virtual cable
    uint8_t sysex_message[max_sysex]; // TODO: make one for every virtual cable for this device
    int sysex_idx;  // the index into the sysex_message array // TODO: make one for every virtual cable for this device

    bool waiting_for_eox;
    void *midi_uart_instance;
    Mc_fader_sync fader_sync[9];
    enum {
        Dev_descriptor, // Getting the Device Descriptor
        Conf_descriptor, // Getting the configuration descriptor
        Langids, // Getting the Language ID list (string descriptor 0)
        String_list, // Getting the string index list
        All_strings, // Getting every string for every supported language 
        Operating    // Normal MIDI I/O operation
    } state;
    std::vector<uint8_t> istrings;
    int istrings_idx;
    std::vector<uint16_t> langids;
    int langids_idx;
    struct Usb_string_s {
        Usb_string_s() : index{0}, length{0}, langid{0}, utf16le{nullptr} {}
        uint8_t index;  // the string index from the USB descriptor
        uint8_t length; // number of 16-bit utf16le character codes in the string
        uint16_t langid; // the language ID associated with this string.
        uint16_t* utf16le; // points to memory allocated to hold the string.
    };
    std::vector<Usb_string_s> string_list;
    static const uint SETTINGS_ENC_SW_GPIO=22;
    static const uint SETTINGS_ENC_A_GPIO=26;
    static const uint SETTINGS_ENC_B_GPIO=27;
    Core1_gpio& core1_gpio;
    int settings_enc_sw_idx;
    int settings_enc_a_idx;
    int settings_enc_b_idx;
    int settings_enc_idx;
    std::vector<int32_t> settings_enc_status;
    bool prev_encoder_sw_pressed;
};
}
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
    settings_file{model},
    setup_menu{screen_tc, screen_tc.get_clip_rect(), model, settings_file},
    seven_seg{tc_view_manager, screen_tc, false, false, setup_menu},
    sysex_idx{0}, waiting_for_eox{false},
    state{Dev_descriptor},
    core1_gpio{Core1_gpio::instance()},
    prev_encoder_sw_pressed{false}
{
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    Pico_pico_midi_lib::instance().init(nullptr, static_cmd_cb);
    // create the settings encoder debounced GPIO input objects
    settings_enc_sw_idx = core1_gpio.add_input(SETTINGS_ENC_SW_GPIO, true);
    settings_enc_a_idx = core1_gpio.add_input(SETTINGS_ENC_A_GPIO, true);
    settings_enc_b_idx = core1_gpio.add_input(SETTINGS_ENC_B_GPIO, true);
    // create the settings encoder position decoder object
    settings_enc_idx = core1_gpio.add_encoder(settings_enc_a_idx, settings_enc_b_idx);
    // reserve the space for the debounced GPIO status
    settings_enc_status.resize(core1_gpio.get_status_size());
    // start debouncing GPIO on core 1
    core1_gpio.start_core1_gpio();
    render_done_mask = 0;
    int num_displays = sizeof(screen)/sizeof(screen[0]);
    uint16_t target_done_mask = (1<<(num_displays+1)) -1;
    bool success = true;
    for (int idx=0; success && idx < sizeof(screen)/sizeof(screen[0]); idx++) {
        screen[idx]->render_non_blocking(callback, idx);
    }
    tc_view_manager.push_view(&seven_seg);
    screen_tc.render_non_blocking(callback, 8);
    while (success && render_done_mask != target_done_mask) {
        for (int idx=0; success && idx < num_displays; idx++) {
            success = screen[idx]->task();
        }
        if (success)
            success = screen_tc.task();
    }
    assert(success);

    // start the reporting of the settings encoder sw and settings encoder positions
    assert(core1_gpio.request_status_update());
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
                // The whole configuration descriptor is read in at this point.
                // Can recall settings for the connected device
                uint16_t vid, pid;
                uint8_t num_rx_cables, num_tx_cables;
                get_vid_pid(&vid,&pid);
                get_num_cables(&num_rx_cables, &num_tx_cables);
                settings_file.set_vid_pid(vid,pid);
                model.set_num_cables(num_rx_cables, num_tx_cables);
                if (!settings_file.load()) {
                    model.load_defaults();
                    if (settings_file.store() < 0) {
                        printf("Error: settings not saved\r\n");
                    }
                    else {
                        model.clear_needs_save();
                    }
                }
                else {
                    model.clear_needs_save();
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
                usb_string.length = length_-3;
                usb_string.utf16le = new uint16_t[usb_string.length];
                memcpy((uint8_t*)usb_string.utf16le, payload_+3, length_-3);
                string_list.push_back(usb_string);
                istrings_idx++;
                if (istrings_idx >= istrings.size()) {
                    istrings_idx = 0;
                    if (++langids_idx >= langids.size()) {
                        // Got all the strings
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
    }
}

void rppicomidi::Pico_mc_display_bridge_dev::request_dev_desc()
{
    rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(REQUEST_DEV_DESC, nullptr, 0);
}

void rppicomidi::Pico_mc_display_bridge_dev::midi_cb(uint8_t *rx, uint8_t buflen, uint8_t cable_num)
{
    static struct {
        uint8_t fader;
        uint8_t byte1;
        uint8_t byte2;
        uint8_t bytes_remaining;
    } last_pitch_bend_message = {0,0,0};
    for (int idx=0; idx<buflen;idx++) {
        uint32_t nwritten = 0;
        uint32_t expected_nwritten = 1;
        if ((rx[idx] & 0xF0) == 0xE0 && (rx[idx]&0xF) < 9) {
            // pitch bend message == Mackie Control fader message
            last_pitch_bend_message.fader = rx[idx]&0xF;
            last_pitch_bend_message.bytes_remaining = 2;
            expected_nwritten = 0;
        }
        else if (last_pitch_bend_message.bytes_remaining == 2) {
            // TODO handle error condition
            last_pitch_bend_message.byte1 = rx[idx];
            last_pitch_bend_message.bytes_remaining = 1;
            expected_nwritten = 0;
        }
        else if (last_pitch_bend_message.bytes_remaining == 1) {
            // TODO handle error condition
            // Got the whole pitch bend message
            last_pitch_bend_message.byte2 = rx[idx];
            last_pitch_bend_message.bytes_remaining = 0;
            Mc_fader_sync::Sync_state state =
                Pico_mc_display_bridge_dev::instance().fader_sync[last_pitch_bend_message.fader].
                        update_fader_position(last_pitch_bend_message.byte1, last_pitch_bend_message.byte2);
            if (state == Mc_fader_sync::Synchronized) {
                expected_nwritten = 3;
                uint8_t message[3] = {0xe0, last_pitch_bend_message.byte1, last_pitch_bend_message.byte2};
                message[0] |= last_pitch_bend_message.fader;
                nwritten = tud_midi_stream_write(cable_num, message, expected_nwritten);
            }
            else {
                // TODO show synchronization GUI
                expected_nwritten = 0;
            }
            TU_LOG2("Sync state fader %d=%s: target=%d fader=%d\r\n",
                last_pitch_bend_message.fader, fader_sync[last_pitch_bend_message.fader].get_state_string(state),
                fader_sync[last_pitch_bend_message.fader].get_target_position(),
                fader_sync[last_pitch_bend_message.fader].get_fader_position());
        }
        else {
            nwritten = tud_midi_stream_write(cable_num, rx+idx, expected_nwritten);
        }
        if (nwritten != expected_nwritten) {
            TU_LOG2("Warning: Dropped %d bytes receiving from UART MIDI In\r\n", expected_nwritten - nwritten);
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
            TU_LOG1("Warning: Dropped %d bytes of host_connection_query message\r\n", sizeof(host_connection_query) - nwritten);
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
            TU_LOG1("Warning: Dropped %d bytes of serial_number_response message\r\n", sizeof(serial_number_response) - nwritten);
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
    uint8_t nread = 0;

    while(tud_midi_n_packet_read(0, rx))
    {
        uint8_t const code_index = rx[0] & 0x0f;
        uint8_t const cable_num = (rx[0] >> 4) & 0xf;

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
        Mc_fader_sync::Sync_state fader_sync_state;
        int fader;
        uint8_t mc_in, mc_out;
        model.get_mc_cable(mc_in, mc_out);
        switch(rx[1]) {
            case 0x90: // Note on (LED control message)
                // Note message, channel 1
                success = seven_seg.set_smpte_beats_by_mc_note(rx[2], rx[3]);
                if (!success) {
                    for (int chan = 0; !success && chan < 8; chan++) {
                        success = channel_disp[chan]->push_midi_message(rx+1, nread);
                    }
                }
                // Pass this on
                if (cable_num == mc_in) {
                    // Remap Mackie Control messages from the DAW to what the connected device understands
                    uint8_t note = rx[2];
                    assert(note < 128);
                    const uint8_t* ptr = model.get_button_mapping();
                    int new_note;
                    for (new_note = 0; new_note < 128 && note != ptr[new_note]; new_note++)
                        ;
                    if (new_note < 128)
                        rx[2] = new_note;
                }
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
                    for (int chan = 0; !success && chan < 8; chan++) {
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
                for (int chan = 0; !success && chan < 8; chan++) {
                    success = channel_disp[chan]->push_midi_message(rx +1, 2);
                }
                // filter these meter messages out
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
                            for (int chan = 0; chan < 8; chan++) {
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
                            for (int kdx=0; kdx<=sysex_idx;kdx++) {
                                printf("%02x ", sysex_message[kdx]);
                            }
                            printf("\n\r");
                        }
                    }
                    waiting_for_eox = false;
                    sysex_idx = 0;
                }
                break;
#if 0
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
                            for (int chan = 0; chan < 8; chan++) {
                                if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                    ++nsuccessful;
                                }
                            }
                            success = nsuccessful != 0;
                        }
                        if (!success) {
                            push_to_midi_uart(sysex_message, sysex_idx+1, cable_num); // send it on to the MIDI UART
                            printf("unhandled:\r\n");
                            for (int kdx=0; kdx<=sysex_idx;kdx++) {
                                printf("%02x ",sysex_message[kdx]);
                            }
                            printf("\n\r");
                        }
                    }
                }
                waiting_for_eox = false;
                sysex_idx = 0;
                break;
#endif
            case 0xE0:  // Pitch bend (fader messages)
            case 0xE1:
            case 0xE2:
            case 0xE3:
            case 0xE4:
            case 0xE5:
            case 0xE6:
            case 0xE7:
            case 0xE8:
                fader = rx[1] &0xf;
                fader_sync_state = fader_sync[fader].update_target_position(rx[2], rx[3]);
                TU_LOG2("From DAW: Fader %u sync state:=%s target=%d fader =%d\r\n",fader, fader_sync[fader].get_state_string(fader_sync_state),
                    fader_sync[fader].get_target_position(), fader_sync[fader].get_fader_position());
                // filter out
                break;
            default:
                if (waiting_for_eox)
                {
                    // Might be the middle of a sysex message or the tail end of one
                    for (jdx = 1; jdx <=nread && (rx[jdx] & 0x80) == 0; jdx++)
                    {
                        sysex_message[sysex_idx++] = rx[jdx];
                    }
                    if ((rx[jdx] & 0x80) == 0)
                    {
                        if (rx[jdx] == 0xF7)
                        {
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
                                    for (int chan = 0; chan < 8; chan++) {
                                        if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                            ++nsuccessful;
                                        }
                                    }
                                    success = nsuccessful != 0;
                                }
                                if (!success) {
                                    push_to_midi_uart(sysex_message, sysex_idx+1, cable_num); // send it on to the MIDI UART
                                    printf("unhandled:\r\n");
                                    for (int kdx=0; kdx<=sysex_idx;kdx++) {
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
                else
                {
                    // Just an unhandled message. Pass it on for the host
                    push_to_midi_uart(rx+1, nread, cable_num);
                }
                break;
        }
        success = false;
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

    if (core1_gpio.get_status_update(settings_enc_status)) {
#if 0
        printf("Encoder delta=%d switch is%s on\r\n", settings_enc_status.at(settings_enc_idx+1),
            (settings_enc_status.at(0) & (1 << settings_enc_sw_idx))?"":" not");
#endif
        int32_t delta = settings_enc_status.at(settings_enc_idx+1);
        if (delta < 0)
            tc_view_manager.on_decrement((uint32_t)(-delta));
        else if (delta > 0)
            tc_view_manager.on_increment((uint32_t)delta);
        bool pressed = (settings_enc_status.at(0) & (1 << settings_enc_sw_idx)) != 0;
        if (pressed != prev_encoder_sw_pressed) {
            prev_encoder_sw_pressed = pressed;
            if (pressed)
                tc_view_manager.on_select();
        }
        assert(core1_gpio.request_status_update());
    }
    // Update the screens if need be
    for (int chan = 0; chan < 8; chan++) {
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
    uint64_t now = time_us_64();
    using namespace rppicomidi;
    Pico_mc_display_bridge_dev::instance().serial_number[0] = now & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[1] = (now >> 8ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[2] = (now >> 16ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[3] = (now >> 24ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[4] = (now >> 32ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[5] = (now >> 40ull) & 0x7Full;
    Pico_mc_display_bridge_dev::instance().serial_number[6] = (now >> 48ull) & 0x7Full;
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
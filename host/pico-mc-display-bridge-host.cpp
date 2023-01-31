/**
 * @file pico-mc-display-bridge-host.c
 * @brief The host adapter and user input adapter for the pico-mc-display-bridge
 * project
 * 
 * MIT License

 * Copyright (c) 2022 rppicomidi

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

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
#include <cstdlib>
#include <vector>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico_pico_midi_lib_config.h"
#include "pico_pico_midi_lib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "class/midi/midi_host.h"

namespace rppicomidi
{
class Pico_mc_display_bridge_host
{
public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Pico_mc_display_bridge_host& instance()
    {
        static Pico_mc_display_bridge_host _instance; // Guaranteed to be destroyed.
                                             // Instantiated on first use.
        return _instance;
    }
    Pico_mc_display_bridge_host(Pico_pico_midi_lib const&) = delete;
    void operator=(Pico_mc_display_bridge_host const&) = delete;

    /**
     * @brief Construct a new Pico_mc_display_bridge_host object
     * 
     */
    Pico_mc_display_bridge_host();

    static void midi_cb(uint8_t *buffer, uint8_t buflen, uint8_t cable_num);
    static void cmd_cb(uint8_t header, uint8_t* buffer, uint16_t length);
    /**
     * @brief Perform periodic actions
     */
    void task();

    /**
     * @brief Periodically blink the LED based on the external
     * MIDI device connection status
     */
    void blink_led();

    void poll_usb_rx(bool connected);

    void poll_midi_uart_rx(bool connected);

    void usbh_langids_cb(uint8_t dev_addr, uint8_t _num_langids, uint16_t* _langids);

    void usbh_dev_string_cb(uint8_t dev_addr, uint8_t istring, uint16_t langid, uint8_t num_utf16le, uint16_t* utf16le);

    uint8_t midi_dev_addr;

    static const uint8_t max_string_indices = 32;
    uint8_t all_string_indices[max_string_indices];
    uint8_t num_strings;
    uint8_t dev_desc_cache[18]; // a memory image of the device descriptor
    uint8_t config_desc_cache[512]; // a memory image of the device configuration descriptor
    enum {Disconnected, Device_setup, Operating} state;
    // Make the command definitions class variables
    #include "../common/pico-mc-display-bridge-cmds.h"
};
} // namespace rppicomidi
// On-board LED mapping. classIf no LED, set to NO_LED_GPIO 
const uint NO_LED_GPIO = 255;
const uint LED_GPIO = 25;

void rppicomidi::Pico_mc_display_bridge_host::midi_cb(uint8_t *buffer, uint8_t buflen, uint8_t cable_num)
{
    if (buflen > 0 && tuh_midih_get_num_tx_cables(instance().midi_dev_addr) >= 1)
    {
        uint32_t nwritten = tuh_midi_stream_write(instance().midi_dev_addr, cable_num,buffer, buflen);
        if (nwritten != buflen) {
            TU_LOG1("Warning: Dropped %d bytes receiving from UART MIDI In\r\n", buflen - nwritten);
        }
    }
}

void rppicomidi::Pico_mc_display_bridge_host::cmd_cb(uint8_t header, uint8_t* payload_, uint16_t length_)
{
    uint8_t payload[256];
    uint8_t length;
    uint8_t dev_addr = instance().midi_dev_addr;
    if (!tuh_mounted(dev_addr) || (instance().state != Operating && instance().state != Device_setup))
        return;
    printf("got command %02x\r\n", header);
    switch(header) {
        case REQUEST_DEV_DESC:
            // send back the whole device descriptor. received payload is ignored
            if (instance().dev_desc_cache[0] == 18) {
                length = sizeof(instance().dev_desc_cache);
                rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(RETURN_DEV_DESC, instance().dev_desc_cache, length);
            }
            break;
        case REQUEST_CONF_DESC_0:
        case REQUEST_CONF_DESC_1:
        case REQUEST_CONF_DESC_2:
        {
            uint16_t total_length = reinterpret_cast<const tusb_desc_configuration_t*>(instance().config_desc_cache)->wTotalLength;
            if (total_length == 0) {
                printf("Have not gotten config descriptor yet\r\n");
                break;
            }
            uint8_t return_header = header + 1;
            uint16_t offset = 0;
            if (header == REQUEST_CONF_DESC_0) {
                length = total_length <=CONFIG_DESC_MAX_PAYLOAD ? total_length : CONFIG_DESC_MAX_PAYLOAD;
            }
            else if (header == REQUEST_CONF_DESC_1) {
                if (total_length <= CONFIG_DESC_MAX_PAYLOAD) {
                    length = 0; // no data to return
                }
                else {
                    uint16_t rem_length = total_length - CONFIG_DESC_MAX_PAYLOAD;
                    length = rem_length <= CONFIG_DESC_MAX_PAYLOAD ? rem_length : CONFIG_DESC_MAX_PAYLOAD;
                }
            }
            else if (header == REQUEST_CONF_DESC_2) {
                if (total_length <= (CONFIG_DESC_MAX_PAYLOAD * 2) ) {
                    length = 0; // no data to return
                }
                else {
                    uint16_t rem_length = total_length - (CONFIG_DESC_MAX_PAYLOAD * 2);
                    length = rem_length <= CONFIG_DESC_MAX_PAYLOAD ? rem_length : CONFIG_DESC_MAX_PAYLOAD;
                }
            }
            rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(return_header+offset, instance().config_desc_cache, length);
        }
            break;
        case REQUEST_DEV_LANGIDS:
            // get the langID list from the device
            // The callback will send it back to Device Pico
            assert(usbh_request_langids(dev_addr));
            break;
        case REQUEST_DEV_STRING_IDXS:
            instance().num_strings = tuh_midi_get_all_istrings(dev_addr, instance().all_string_indices, sizeof(all_string_indices));
            length = instance().num_strings <= 255 ? instance().num_strings : 255;
            rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(RETURN_DEV_STRING_IDXS, 
                instance().all_string_indices, length);
            break;
        case REQUEST_DEV_STRING:
        {
            // TODO: move this to string callback
            uint8_t istring = payload_[0];
            uint16_t langid = payload_[1] | (payload_[2] << 8);
            assert(usbh_request_device_string(dev_addr, istring, langid));
        }
            break;
        case RESYNCHRONIZE:
            if (instance().state == Device_setup) {
                instance().state = Operating;
            }
            break;
        default:
            break;
    }
}

rppicomidi::Pico_mc_display_bridge_host::Pico_mc_display_bridge_host() :
  midi_dev_addr{0}, num_strings{0}, state{Disconnected}
{
    Pico_pico_midi_lib::instance().init(midi_cb, cmd_cb); // instantiate the Pico_pico_midi_lib class object and initialize it
    // Map the pins to functions
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    memset(dev_desc_cache, 0, sizeof(dev_desc_cache));
    memset(config_desc_cache, 0, sizeof(config_desc_cache));
}

void rppicomidi::Pico_mc_display_bridge_host::blink_led()
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

void rppicomidi::Pico_mc_display_bridge_host::poll_usb_rx(bool connected)
{
    // device must be attached and have at least one endpoint ready to receive a message
    if (!connected || tuh_midih_get_num_rx_cables(midi_dev_addr) < 1)
    {
        return;
    }
    tuh_midi_read_poll(midi_dev_addr);
}


void rppicomidi::Pico_mc_display_bridge_host::poll_midi_uart_rx(bool connected)
{
    if (connected) {
        rppicomidi::Pico_pico_midi_lib::instance().poll_rx_buffer();
    }
}

void rppicomidi::Pico_mc_display_bridge_host::task()
{
    blink_led();
    bool connected = midi_dev_addr != 0 && tuh_midi_configured(midi_dev_addr);
    if (connected) {
        switch (state) {
            case Disconnected:
                state = Device_setup; // Devinfo;
                break;
            case Device_setup:
                poll_midi_uart_rx(connected);
                rppicomidi::Pico_pico_midi_lib::instance().drain_tx_buffer();
                break;
            case Operating:
                poll_midi_uart_rx(connected);
                if (connected)
                    tuh_midi_stream_flush(midi_dev_addr);
                poll_usb_rx(connected);
                rppicomidi::Pico_pico_midi_lib::instance().drain_tx_buffer();
                break;
            default:
                printf("illegal state\r\n");
                for (;;) ;
                break;
        }
    }
    else {
        state = Disconnected;
    }
}

void rppicomidi::Pico_mc_display_bridge_host::usbh_langids_cb(uint8_t dev_addr, uint8_t _num_langids, uint16_t* _langids)
{
    if (midi_dev_addr == dev_addr)
    {
        if (_num_langids != 0 && _langids != nullptr) {
            assert(rppicomidi::Pico_pico_midi_lib::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(RETURN_DEV_LANGIDS,
                (uint8_t*)_langids, _num_langids*2)==_num_langids*2);
        }
    }
}

int main() {
    using namespace rppicomidi;
    bi_decl(bi_program_description("Provide a USB host interface for Serial Port MIDI."));
    bi_decl(bi_1pin_with_name(LED_GPIO, "On-board LED"));
    bi_decl(bi_2pins_with_names(PICO_PICO_MIDI_LIB_UART_TX_GPIO, "MIDI UART TX", PICO_PICO_MIDI_LIB_UART_RX_GPIO, "MIDI UART RX"));

    board_init();
    printf("Pico Mackie Control Display Bridge MIDI Host\r\n");
    Pico_mc_display_bridge_host::instance();
    tusb_init();

    while(1) {
        tuh_task();
        Pico_mc_display_bridge_host::instance().task();
    }
}

void rppicomidi::Pico_mc_display_bridge_host::usbh_dev_string_cb(uint8_t dev_addr, uint8_t istring, uint16_t langid, uint8_t num_utf16le, uint16_t* utf16le)
{
    if (midi_dev_addr == dev_addr && num_utf16le != 0 && utf16le != NULL)
    {
        int test = num_utf16le*2 + 3;
        uint8_t length = 255;
        if (test < 256)
            length = test;

        uint8_t payload[255];
        payload[0] = istring;
        payload[1] = langid & 0xff;
        payload[2] = (langid >> 8) & 0xff;
        memcpy(payload+3, (uint8_t*)utf16le, length -3);
        rppicomidi::Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(RETURN_DEV_STRING, payload, length);
    }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint16_t num_cables_tx)
{
  printf("MIDI device address = %u, IN endpoint %u has %u cables, OUT endpoint %u has %u cables\r\n",
      dev_addr, in_ep & 0xf, num_cables_rx, out_ep & 0xf, num_cables_tx);

  rppicomidi::Pico_mc_display_bridge_host::instance().midi_dev_addr = dev_addr;
}

// Invoked when device with hid interface is un-mounted
void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  rppicomidi::Pico_mc_display_bridge_host::instance().midi_dev_addr = 0;
  printf("MIDI device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets)
{
    if (rppicomidi::Pico_mc_display_bridge_host::instance().midi_dev_addr == dev_addr)
    {
        if (num_packets != 0)
        {
            uint8_t cable_num;
            uint8_t buffer[48];
            uint32_t bytes_read = tuh_midi_stream_read(dev_addr, &cable_num, buffer, sizeof(buffer));
            uint8_t npushed = rppicomidi::Pico_pico_midi_lib::instance().write_midi_to_tx_buffer(buffer, bytes_read, cable_num); //rppicomidi::Pico_pico_midi_lib::instance().write_tx_buffer(buffer,bytes_read);
            if (npushed != bytes_read) {
                TU_LOG1("Warning: Dropped %d bytes sending to UART MIDI Out\r\n", bytes_read - npushed);
            }
        }
    }
}

void tuh_midi_tx_cb(uint8_t dev_addr)
{

}

void usbh_langids_cb(uint8_t dev_addr, uint8_t _num_langids, uint16_t* _langids)
{
    rppicomidi::Pico_mc_display_bridge_host::instance().usbh_langids_cb(dev_addr, _num_langids, _langids);
}

void usbh_dev_string_cb(uint8_t dev_addr, uint8_t istring, uint16_t langid, uint8_t num_utf16le, uint16_t* utf16le)
{
    rppicomidi::Pico_mc_display_bridge_host::instance().usbh_dev_string_cb(dev_addr, istring, langid, num_utf16le, utf16le);
}


void tuh_attach_cb (tusb_desc_device_t const *desc_device_)
{
    printf("Got device descriptor %u bytes\r\n", desc_device_->bLength);
    assert(desc_device_->bLength == sizeof(rppicomidi::Pico_mc_display_bridge_host::instance().dev_desc_cache));
    memcpy(rppicomidi::Pico_mc_display_bridge_host::instance().dev_desc_cache, (uint8_t*)desc_device_, desc_device_->bLength);
}

void tuh_configuration_parsed_cb(uint8_t dev_addr, const tusb_desc_configuration_t* raw_config_desc)
{
    printf("Got configuration descriptor %u bytes\n\r", raw_config_desc->wTotalLength);
    assert(raw_config_desc->wTotalLength <= sizeof(rppicomidi::Pico_mc_display_bridge_host::instance().config_desc_cache));
    memcpy(rppicomidi::Pico_mc_display_bridge_host::instance().config_desc_cache, (uint8_t*)raw_config_desc, raw_config_desc->wTotalLength);
}
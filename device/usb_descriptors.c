/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

//#include "tusb.h"
#include "usb_descriptors.h"
#include "../common/pico-mc-display-bridge-cmds.h"
#include <stdlib.h>

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// This descriptor will be overwritten
static tusb_desc_device_t desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xdead,
    .idProduct          = 0xbeef,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

void get_vid_pid(uint16_t* vid, uint16_t* pid)
{
    *vid = desc_device.idVendor;
    *pid = desc_device.idProduct;
}

void set_dev_desc(uint8_t* desc_cache)
{
    memcpy((uint8_t*)&desc_device, desc_cache, sizeof(desc_device));
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}


//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

static uint8_t* desc_fs_configuration = NULL;

#if TUD_OPT_HIGH_SPEED
static uint8_t* desc_hs_configuration = NULL;
#endif

uint16_t get_midi_configuration_length()
{
    tusb_desc_configuration_t* conf_desc=(tusb_desc_configuration_t*)desc_fs_configuration;
    uint16_t total_length = 0;
    if (conf_desc) {
        total_length = conf_desc->wTotalLength;
    }
    return total_length;
}

bool create_midi_configuration_descriptors(uint8_t* desc_cache)
{
    tusb_desc_configuration_t* conf_desc=(tusb_desc_configuration_t*)desc_cache;
    bool success = false;
    if (desc_cache != NULL && conf_desc->wTotalLength < CONFIG_DESC_MAX_PAYLOAD*3) { // We can't read in a descriptor longer than that
        desc_fs_configuration = malloc(conf_desc->wTotalLength);
        if (desc_fs_configuration) {
            memcpy(desc_fs_configuration, desc_cache, conf_desc->wTotalLength <= CONFIG_DESC_MAX_PAYLOAD ? conf_desc->wTotalLength : CONFIG_DESC_MAX_PAYLOAD);
            success = true;
        }
    }
    return success;
}

bool append_midi_configuration_descriptors(uint8_t* desc_cache, uint16_t offset, uint8_t length)
{
    bool success = false;
    if (desc_cache && offset >=255 && desc_fs_configuration != NULL) {
        tusb_desc_configuration_t* conf_desc=(tusb_desc_configuration_t*)desc_fs_configuration;
        if ((length + offset) <= conf_desc->wTotalLength) {
            memcpy(desc_fs_configuration+offset, desc_cache, length);
            success = true;
        }
    }
    return success;
}

void get_num_cables(uint8_t* num_rx_cables, uint8_t* num_tx_cables)
{
    // extract the number of cables in and out from the configuration
    *num_rx_cables = 0;
    *num_tx_cables = 0;
    const tusb_desc_endpoint_t* desc_ept = (const tusb_desc_endpoint_t*)desc_fs_configuration;
    uint32_t max_addr = (uint32_t) desc_fs_configuration + ((const tusb_desc_configuration_t*)desc_fs_configuration)->wTotalLength;
    int num_endpoints;
    for (num_endpoints = 0; num_endpoints < 2; num_endpoints++) {
        while ((uint32_t)desc_ept < max_addr && desc_ept->bDescriptorType != TUSB_DESC_ENDPOINT) {
            desc_ept = (const tusb_desc_endpoint_t*)tu_desc_next(desc_ept);
        }
        if ( (uint32_t)desc_ept < max_addr) {
            // found an endpoint. Find the MIDIStreaming Endpoint descriptor
            const midi_cs_desc_endpoint_t* mcsed = (const midi_cs_desc_endpoint_t*)tu_desc_next(desc_ept);
            while ((uint32_t)mcsed < max_addr && (mcsed->bDescriptorType != TUSB_DESC_CS_ENDPOINT || mcsed->bDescriptorSubType !=  MIDI_CS_ENDPOINT_GENERAL)) {
                mcsed = (const midi_cs_desc_endpoint_t*)tu_desc_next(desc_ept);
            }
            if ( (uint32_t)desc_ept < max_addr) {
                if ( (desc_ept->bEndpointAddress & 0x80)  == 0) {
                    *num_rx_cables = mcsed->bNumEmbMIDIJack;
                }
                else {
                    *num_tx_cables = mcsed->bNumEmbMIDIJack;
                }
                desc_ept = (const tusb_desc_endpoint_t*)tu_desc_next(desc_ept);
            }
        }
    }
}
// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

#if TUD_OPT_HIGH_SPEED
  // Although we are highspeed, host may be fullspeed.
  return (tud_speed_get() == TUSB_SPEED_HIGH) ?  desc_hs_configuration : desc_fs_configuration;
#else
  return desc_fs_configuration;
#endif
}

uint8_t get_product_string_index()
{
    return desc_device.iProduct;
}
//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
// callbacked implemented in the main application because data is stored there
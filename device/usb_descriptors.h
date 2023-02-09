/**
 * @file usb_descriptors.h
 * @brief This file describes the API for dynamically creating a
 * USB MIDI device descriptor with up to 16 cables
 *
 * Copyright (c) 2022 rppicomidi
 *
 * The MIT License (MIT)
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
 */

#pragma once
#include "tusb.h"
#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief Set the device descriptor to the 18-byte array desc_cache
 *
 * @param desc_cache an array containing the device descriptor
 */
void set_dev_desc(uint8_t* desc_cache);

void get_vid_pid(uint16_t* vid, uint16_t* pid);

/**
 * @brief Allocate memory for the configuration descriptors and copy either
 * wTotalLength bytes (value from the descriptor itself) or the first 
 * CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor, whichever is less
 *
 * @param desc_cache is the first of up to CONFIG_DESC_MAX_PAYLOAD bytes of the
 * configuration descriptor
 * @return true if successfully allocated memory and copied the descriptor.
 * @return false otherwise
 */
bool create_midi_configuration_descriptors(uint8_t* desc_cache);

/**
 * @brief copy length bytes to the configuration descriptor from the desc_cache
 * starting at configuration descriptor offset
 *
 * @param desc_cache up to CONFIG_DESC_MAX_PAYLOAD bytes of configuration descriptor
 * (but not the first block; offset can't be 0)
 * @param offset where in the configuration descriptor to start copying. Should be
 * a multiple of CONFIG_DESC_MAX_PAYLOAD
 * @param length the number of bytes to copy
 * @return true if successful
 * @return false otherwise
 */
bool append_midi_configuration_descriptors(uint8_t* desc_cache, uint16_t offset, uint8_t length);

/**
 * @brief Get the midi configuration descriptor length
 *
 * @return wTotalLength from the descriptor
 */
uint16_t get_midi_configuration_length();

/**
 * @brief Get the number of cables in and out from the configuration descriptor
 *
 * @param num_rx_cables number of OUT endpoint embedded jacks
 * @param num_tx_cables number of IN endpoint embedded jacks
 *
 * @note values can be 0
 */
void get_num_cables(uint8_t* num_rx_cables, uint8_t* num_tx_cables);

/**
 * @brief Get the product string index
 * 
 * @return 0 if no product string, or the product string index if there is one
 */
uint8_t get_product_string_index();
#ifdef __cplusplus
}
#endif
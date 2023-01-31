/**
 * @file pico-mc-display-bridge-cmds.h
 * @brief include this file in the implementation for the host
 *        and the device Pico
 *
 * MIT License
 *
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
 */


#pragma once

    static const uint8_t REQUEST_DEV_DESC=0x40;
    static const uint8_t RETURN_DEV_DESC=0x41;
    static const uint8_t REQUEST_DEV_STRING_IDXS=0x42;
    static const uint8_t RETURN_DEV_STRING_IDXS=0x43;
    static const uint8_t REQUEST_DEV_LANGIDS=0x44;
    static const uint8_t RETURN_DEV_LANGIDS=0x45;
    static const uint8_t REQUEST_DEV_STRING=0x46;
    static const uint8_t RETURN_DEV_STRING=0x47;
    static const uint8_t RESYNCHRONIZE=0x48;
    // The configuration descriptor may be too long to send back in one go
    static const uint8_t REQUEST_CONF_DESC_0=0x49; // Request the first up to CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor
    static const uint8_t RETURN_CONF_DESC_0=0x4A;  // Return the first up to CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor
    static const uint8_t REQUEST_CONF_DESC_1=0x4B; // Request the second up to CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor
    static const uint8_t RETURN_CONF_DESC_1=0x4C;  // Return the second up to CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor
    static const uint8_t REQUEST_CONF_DESC_2=0x4D; // Request the third up to CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor
    static const uint8_t RETURN_CONF_DESC_2=0x4E;  // Return the third up to CONFIG_DESC_MAX_PAYLOAD bytes of the descriptor
    static const uint8_t CONFIG_DESC_MAX_PAYLOAD=200; // The largest payload that will be returned in a RETURN_CONF_DESC_? message

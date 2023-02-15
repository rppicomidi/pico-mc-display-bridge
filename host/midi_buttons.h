/**
 * MIT License
 *
 * Copyright (c) 2023 rppicomidi
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
#include <cstdint>
#include "pico_pico_midi_lib.h"
// Button GP number default definitions; assumption is that the set of 7 buttons
// use a continuous group of 7 GPIO pins. The order of the pins and the first
// pin number is controlled by the following preprocessor defines
#ifndef CHAN_BTN_FIRST_GPIO
#define CHAN_BTN_FIRST_GPIO 13
#endif
#ifndef CHAN_BTN_1
#define CHAN_BTN_1 13
#endif
#ifndef CHAN_BTN_2
#define CHAN_BTN_2 14
#endif
#ifndef CHAN_BTN_3
#define CHAN_BTN_3 15
#endif
#ifndef CHAN_BTN_4
#define CHAN_BTN_4 16
#endif
#ifndef CHAN_BTN_5
#define CHAN_BTN_5 17
#endif
#ifndef CHAN_BTN_6
#define CHAN_BTN_6 18
#endif
#ifndef CHAN_BTN_7
#define CHAN_BTN_7 19
#endif
#ifndef CHAN_BTN_8
#define CHAN_BTN_8 20
#endif
#ifndef SOLO_BTN
#define SOLO_BTN 21
#endif
#ifndef MUTE_BTN
#define MUTE_BTN 22
#endif
#ifndef REC_BTN
#define REC_BTN 26
#endif
#ifndef VPOT_BTN
#define VPOT_BTN 27
#endif
#ifndef BEATS_SMPTE_BTN
#define BEATS_SMPTE_BTN 3
#endif
#ifndef NAME_VALUE_BTN
#define NAME_VALUE_BTN 2
#endif
namespace rppicomidi {
class Midi_buttons
{
public:
    Midi_buttons();
    void poll();
    void set_cable_number(uint8_t cable_num_) {cable_num = cable_num_; }
private:
    static const uint8_t ndebounce=10;
    uint32_t debounce[ndebounce];
    uint32_t prev_buttons;
    absolute_time_t previous_timestamp;
    uint8_t cable_num;
    uint8_t chan_button_mode;
    static const uint32_t chan_buttons=0x1FE000; // bits 13-20
    static const uint32_t chan_mode_buttons = 0xC600000; //bits 27, 26, 22, 21
    static const uint32_t beats_smpte_button = (1 << BEATS_SMPTE_BTN);
    static const uint32_t name_value_button = (1 << NAME_VALUE_BTN);
    static const uint32_t all_midi_buttons = (chan_buttons | chan_mode_buttons | beats_smpte_button | name_value_button);
    static constexpr const uint8_t chan_btn_msg[5] = {0x18 /*select*/, 0x8 /*solo*/, 0x10 /*mute*/, 0x00 /*rec*/, 0x20 /*vpot*/};
};
}
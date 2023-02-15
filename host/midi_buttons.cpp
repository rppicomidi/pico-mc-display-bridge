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
#include "pico/stdlib.h"
#include "midi_buttons.h"
#include <cstring>
#include "../common/pico-mc-display-bridge-cmds.h"
rppicomidi::Midi_buttons::Midi_buttons() :
    prev_buttons{0}, previous_timestamp{get_absolute_time()}, cable_num{0}, chan_button_mode{0}
{
    // Set up the button GPIO
    gpio_init(CHAN_BTN_1);
    gpio_init(CHAN_BTN_2);
    gpio_init(CHAN_BTN_3);
    gpio_init(CHAN_BTN_4);
    gpio_init(CHAN_BTN_5);
    gpio_init(CHAN_BTN_6);
    gpio_init(CHAN_BTN_7);
    gpio_init(CHAN_BTN_8);
    gpio_init(SOLO_BTN);
    gpio_init(MUTE_BTN);
    gpio_init(REC_BTN);
    gpio_init(VPOT_BTN);
    gpio_init(BEATS_SMPTE_BTN);
    gpio_init(NAME_VALUE_BTN);
    gpio_set_dir(CHAN_BTN_1, GPIO_IN);
    gpio_set_dir(CHAN_BTN_2, GPIO_IN);
    gpio_set_dir(CHAN_BTN_3, GPIO_IN);
    gpio_set_dir(CHAN_BTN_4, GPIO_IN);
    gpio_set_dir(CHAN_BTN_5, GPIO_IN);
    gpio_set_dir(CHAN_BTN_6, GPIO_IN);
    gpio_set_dir(CHAN_BTN_7, GPIO_IN);
    gpio_set_dir(CHAN_BTN_8, GPIO_IN);
    gpio_set_dir(SOLO_BTN, GPIO_IN);
    gpio_set_dir(MUTE_BTN, GPIO_IN);
    gpio_set_dir(REC_BTN, GPIO_IN);
    gpio_set_dir(VPOT_BTN, GPIO_IN);
    gpio_set_dir(BEATS_SMPTE_BTN, GPIO_IN);
    gpio_set_dir(NAME_VALUE_BTN, GPIO_IN);
    gpio_pull_up(CHAN_BTN_1);
    gpio_pull_up(CHAN_BTN_2);
    gpio_pull_up(CHAN_BTN_3);
    gpio_pull_up(CHAN_BTN_4);
    gpio_pull_up(CHAN_BTN_5);
    gpio_pull_up(CHAN_BTN_6);
    gpio_pull_up(CHAN_BTN_7);
    gpio_pull_up(CHAN_BTN_8);
    gpio_pull_up(SOLO_BTN);
    gpio_pull_up(MUTE_BTN);
    gpio_pull_up(REC_BTN);
    gpio_pull_up(VPOT_BTN);
    gpio_pull_up(BEATS_SMPTE_BTN);
    gpio_pull_up(NAME_VALUE_BTN);

    memset(debounce, 0, sizeof(debounce));
}

void rppicomidi::Midi_buttons::poll()
{
    absolute_time_t now = get_absolute_time();
    
    int64_t diff = absolute_time_diff_us(previous_timestamp, now);
    // poll no more often than once per millisecond
    if (diff < 1000)
        return;
    previous_timestamp = now;
    uint32_t buttons = ((~gpio_get_all()) & all_midi_buttons);
    bool still_bouncing = (buttons != debounce[0]);
    for (int idx=1; idx < ndebounce; idx ++) {
        still_bouncing = still_bouncing || (buttons != debounce[idx]);
        debounce[idx-1] = debounce[idx];
    }
    debounce[ndebounce-1] = buttons;
    uint8_t tx[3] = {0x90, 0x00, 0x00};
    if (!still_bouncing) {
        uint32_t delta = (prev_buttons ^ buttons) & all_midi_buttons;
        if (delta) {
            prev_buttons = buttons;
        }
        uint32_t diff = delta & chan_mode_buttons;
        if (diff != 0 && (buttons & chan_mode_buttons) != 0) {
            if (buttons & (1 << SOLO_BTN)) {
                if (chan_button_mode != MC_BTN_FN_SOLO)
                    chan_button_mode = MC_BTN_FN_SOLO;
                else
                    chan_button_mode = MC_BTN_FN_SEL;
            }
            else if (buttons & (1 << MUTE_BTN)) {
                if (chan_button_mode != MC_BTN_FN_MUTE)
                    chan_button_mode = MC_BTN_FN_MUTE;
                else
                    chan_button_mode = MC_BTN_FN_SEL;
            }
            else if (buttons & (1 << REC_BTN)) {
                if (chan_button_mode != MC_BTN_FN_REC)
                    chan_button_mode = MC_BTN_FN_REC;
                else
                    chan_button_mode = MC_BTN_FN_SEL;
            }
            else if (buttons & (1 << VPOT_BTN)) {
                if (chan_button_mode != MC_BTN_FN_VPOT)
                    chan_button_mode = MC_BTN_FN_VPOT;
                else
                    chan_button_mode = MC_BTN_FN_SEL;
            }
            Pico_pico_midi_lib::instance().write_cmd_to_tx_buffer(RETURN_MC_BNT_FN,&chan_button_mode, 1);
        }
        diff = delta & chan_buttons;
        if (diff) {
            // channel buttons have changed
            uint32_t mask = (1 << CHAN_BTN_FIRST_GPIO);
            for (uint8_t chan = 0; chan < 8; chan++) {
                if (mask & diff) {
                    if (mask & buttons) {
                        tx[2] = 0x7f; // button is pressed
                    }
                    else {
                        tx[2] = 0x0; // button is released
                    }
                    tx[1] = chan + chan_btn_msg[chan_button_mode];
                    Pico_pico_midi_lib::instance().write_midi_to_tx_buffer(tx, 3, cable_num);
                }
                mask <<= 1;
            }
        }
        diff = delta & beats_smpte_button;
        if (diff) {
            tx[1] = 0x35; // BEATS/SMPTE note number
            if (beats_smpte_button & buttons) {
                tx[2] = 0x7f; // button is pressed
            }
            else {
                tx[2] = 0x0; // button is released
            }
            Pico_pico_midi_lib::instance().write_midi_to_tx_buffer(tx, 3, cable_num);
        }
        diff = delta & name_value_button;
        if (diff) {
            tx[1] = 0x34; // NAME/VALUE note number
            if (name_value_button & buttons) {
                tx[2] = 0x7f; // button is pressed
            }
            else {
                tx[2] = 0x0; // button is released
            }
            Pico_pico_midi_lib::instance().write_midi_to_tx_buffer(tx, 3, cable_num);
        }
    }
}
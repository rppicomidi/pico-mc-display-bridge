/**
 * @file nav_buttons.cpp
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
#include <cstdio>
#include <cstdint>
#include <cstring>
#include "pico/stdlib.h"
#include "nav_buttons.h"

rppicomidi::Nav_buttons::Nav_buttons() :
    prev_buttons{0}, previous_timestamp{get_absolute_time()},
    held_buttons_timeout{0},max_button_repeat_interval_ms{400}, button_repeat_interval_ms{max_button_repeat_interval_ms},
    acceleration_count{10}
{
    // Set up the button GPIO
    gpio_init(BUTTON_UP);
    gpio_init(BUTTON_DOWN);
    gpio_init(BUTTON_LEFT);
    gpio_init(BUTTON_RIGHT);
    gpio_init(BUTTON_ENTER);
    gpio_init(BUTTON_BACK);
    gpio_init(BUTTON_SHIFT);
    gpio_set_dir(BUTTON_UP, GPIO_IN);
    gpio_set_dir(BUTTON_DOWN, GPIO_IN);
    gpio_set_dir(BUTTON_LEFT, GPIO_IN);
    gpio_set_dir(BUTTON_RIGHT, GPIO_IN);
    gpio_set_dir(BUTTON_ENTER, GPIO_IN);
    gpio_set_dir(BUTTON_BACK, GPIO_IN);
    gpio_set_dir(BUTTON_SHIFT, GPIO_IN);
    gpio_pull_up(BUTTON_UP);
    gpio_pull_up(BUTTON_DOWN);
    gpio_pull_up(BUTTON_LEFT);
    gpio_pull_up(BUTTON_RIGHT);
    gpio_pull_up(BUTTON_ENTER);
    gpio_pull_up(BUTTON_BACK);
    gpio_pull_up(BUTTON_SHIFT);
    memset(debounce, 0, sizeof(debounce));
}

uint8_t rppicomidi::Nav_buttons::poll()
{
    uint8_t ret_bitmap = 0;
    absolute_time_t now = get_absolute_time();
    
    int64_t diff = absolute_time_diff_us(previous_timestamp, now);
    // poll no more often than once per millisecond
    if (diff < 1000)
        return ret_bitmap;
    previous_timestamp = now;
    uint8_t buttons = (uint8_t)(~(gpio_get_all() >> BUTTON_FIRST_GPIO) & 0x7f);
    bool still_bouncing = (buttons != debounce[0]);
    for (int idx=1; idx < ndebounce; idx ++) {
        still_bouncing = still_bouncing || (buttons != debounce[idx]);
        debounce[idx-1] = debounce[idx];
    }
    debounce[ndebounce-1] = buttons;

    if (!still_bouncing) {
        if (buttons != prev_buttons || held_buttons_timeout <= 1) {
            if (held_buttons_timeout <=1) {
                if (--acceleration_count <=0) {
                    acceleration_count = 10;
                    button_repeat_interval_ms -= 100;
                    if (button_repeat_interval_ms < 100)
                        button_repeat_interval_ms = 100;
                }
            }
            ret_bitmap = (buttons || prev_buttons) ? (buttons | 0x80) : 0; // bit 7 set means change
            prev_buttons = buttons;
        }
        if (--held_buttons_timeout <= 0) {
            held_buttons_timeout = button_repeat_interval_ms;
        }
    }
    else {
        button_repeat_interval_ms = max_button_repeat_interval_ms;
        held_buttons_timeout = button_repeat_interval_ms;
        acceleration_count = 10;
    }
    return ret_bitmap;
}

const char* rppicomidi::Nav_buttons::get_button_name(uint8_t button_map)
{
    for (int bit = 0; bit < 7; bit++) {
        uint8_t mask = 1 << bit;
        if (mask & button_map) {
            uint8_t button = bit + 6;
            switch (button) {
            case BUTTON_UP:
                return "UP";
            case BUTTON_DOWN:
                return "DOWN";
            case BUTTON_LEFT:
                return "LEFT";
            case BUTTON_RIGHT:
                return "RIGHT";
            case BUTTON_SHIFT:
                return "HOME";
            case BUTTON_BACK:
                return "BACK";
            case BUTTON_ENTER:
                return "ENTER";
            default:
                return "UNKNONW";
            }
        }
    }
    return "NONE PRESSED";
}
/**
 * @file mc_meter.cpp
 * @brief This class implements drawing a vertical bar graph meter
 * that supports Mackie Control meter display
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

#include "mc_meter.h"
#include "pico/time.h"

rppicomidi::Mc_meter::Mc_meter(Mono_graphics& screen_, uint8_t x_, uint8_t y_, uint8_t meter_channel_) :
        screen{screen_}, x{x_}, y{y_}, meter_channel{meter_channel_}, value{0}, overload{false}
{
    time_last_value_set = get_absolute_time();
    draw();
}

void rppicomidi::Mc_meter::draw()
{
    screen.draw_rectangle(x,y, 8, 8, Pixel_state::PIXEL_ONE, overload ? Pixel_state::PIXEL_ONE:Pixel_state::PIXEL_ZERO);
    for (int idx = 0; idx < 12; idx++) {
        Pixel_state fill = (value > (11-idx)) ? Pixel_state::PIXEL_ONE:Pixel_state::PIXEL_ZERO;
        screen.draw_rectangle(x,7+y+idx*7, 8, 8, Pixel_state::PIXEL_ONE, fill);
    }
}

void rppicomidi::Mc_meter::set_value(uint8_t value_, bool overload_)
{
    value = value_;
    overload = overload_;
    time_last_value_set = get_absolute_time();
    draw();
}

void rppicomidi::Mc_meter::set_value_by_channel_pressure(uint8_t message)
{
    uint8_t val = message & 0xF;
    if (val == 0xF) {
        clear_overload();
    }
    else if (val == 0xE) {
        // set overload and maximum level
        set_value(0xC, true);
    }
    else if (val <= 0xC) {
        set_value(val, overload);
    }
    // val == 0xD is undefined. Making it do nothing at all
}

void rppicomidi::Mc_meter::task()
{
    if (value > 0) {
        absolute_time_t now = get_absolute_time();
        
        int64_t diff = absolute_time_diff_us(time_last_value_set, now);
        if (diff > 300000 /* 300 ms */) {
            --value;
            draw();
        }
    }
}
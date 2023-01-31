/**
 * @file mc_meter.h
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
#pragma once
#include "mono_graphics_lib.h"
#include "drawable.h"
#include "pico/stdlib.h"
namespace rppicomidi {
class Mc_meter : public Drawable
{
public:
    Mc_meter(Mono_graphics& screen_, uint8_t x_, uint8_t y_, uint8_t meter_channel_);

    virtual ~Mc_meter() = default;

    /**
     * @brief Set the current meter value and restart the decay timer
     * 
     * @param value is the meter value 0 -12. Value 13 is the same as 12.
     * Value 14 is the same as 12. Value 15 or greater is the same as 0.
     * 
     * @param overload is true if the overload flag should be set
     */
    void set_value(uint8_t value, bool overload);

    /**
     * @brief clear the overload flag and redraw
     * 
     */
    void clear_overload() { overload = false; draw(); }

    /**
     * @brief Set the value by channel pressure object
     * 
     * @param message bits 0 ccc xxxx where
     * ccc is the channel number 0-7,
     * xxxx 0-12 is the meter value
     * xxxx 13 is the same as 12
     * xxxx 14 is the same as 12 plus the overload flag is set
     * xxxx 15 is the same as 0 plus the overload flag is cleared 
     */
    void set_value_by_channel_pressure(uint8_t message);

    /**
     * @brief draw the meter to the screen buffer
     * 
     */
    void draw() final;

    /**
     * @brief handle meter level decay.
     *
     * @note This function should be called
     * frequently enough so that it looks like meter segments decay
     * once per 300ms.
     */
    void task();
private:
    Mc_meter() = delete;
    Mc_meter(Mc_meter&) = delete;

    Mono_graphics& screen;
    uint8_t x,y;
    uint8_t meter_channel;
    uint8_t value;
    bool overload;
    absolute_time_t time_last_value_set;
};
}
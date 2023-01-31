/**
 * @file vpot_display.h
 * @brief This class implements drawing the "LEDs" surrounding a VPot
 * There are 11 value "LEDs" and and one "p" at the bottom center of
 * the display
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
namespace rppicomidi {

enum class Vpot_mode {
    SINGLE_DOT=0,
    BOOST_CUT=1,
    WRAP=2,
    SPREAD=3,
};

class Mc_vpot_display : public Drawable
{
public:
    Mc_vpot_display(Mono_graphics& screen_, uint8_t x_, uint8_t y_, Vpot_mode initial_mode_, uint8_t initial_value_, bool initial_p_);
    virtual ~Mc_vpot_display() = default;

    void draw() final;
    uint8_t get_width() {return width;}
    uint8_t get_height() {return height;}
    void set_mode_and_value(Vpot_mode mode_, uint8_t value_) {
        mode = mode_; value = value_; draw();
    }
    void set_p(bool is_on) { p_led_on = is_on; draw(); }

    /**
     * @brief Set the by Mackie Control VPot LED CC message value
     * 
     * @param cc_value (0 p xx vvvv), where p is 1 to light the p "LED"
     * xx is the Vpot_mode value, and vvvv is the numerical value 0-11.
     * If vvvv is > 11, it will be displayed as 0.
     */
    void set_by_cc_value(uint8_t cc_value);
protected:
    Mc_vpot_display() = delete;
    Mc_vpot_display(Mc_vpot_display&) = delete;

    Mono_graphics& screen;
    uint8_t x0, y0;
    const uint8_t led_r; // = 3;
    const uint8_t outline_r; // = 12;
    const uint8_t led_placement_r;//  = outline_r+ vpot_led_r + 7;
    const uint8_t p_led_placement_r; // center_y+vpot_outline_r+4;
    const uint8_t width; // (led_placement_r + led_r)*2;
    const uint8_t height; // led_placement_r + p_led_placement_r + 2*led_r;
    uint8_t center_x; // = 24;
    uint8_t center_y; // = 75;
    Vpot_mode mode; // how to display the values on the main 11 VPot "LEDs"
    uint8_t value;  // the value 0-11
    bool p_led_on;  // the bottom center "LED" state
};

} // namespace rppicomidi

/**
 * @file vpot_display.cpp
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
 * SOFTWARE. * 
 */

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "mc_vpot_display.h"

rppicomidi::Mc_vpot_display::Mc_vpot_display(Mono_graphics& screen_, uint8_t x_, uint8_t y_, Vpot_mode initial_mode_, 
        uint8_t initial_value_, bool initial_p_) :
    screen{screen_}, x0{x_}, y0{y_}, led_r{3}, outline_r{12}, led_placement_r{(uint8_t)(outline_r+ led_r + 7)}, p_led_placement_r{(uint8_t)(outline_r+led_r+1)},
    width{(uint8_t)((led_placement_r + led_r)*2)}, height{(uint8_t)(led_placement_r + p_led_placement_r + 2*led_r)}, 
    center_x{(uint8_t)(x_+width/2)}, center_y{(uint8_t)(y_+height/2)},
    mode{initial_mode_}, value{initial_value_}, p_led_on{initial_p_}
{
    draw();
}

void rppicomidi::Mc_vpot_display::draw()
{
    screen.draw_centered_circle(center_x, center_y, outline_r, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_TRANSPARENT); // outline
    screen.draw_centered_circle(center_x, center_y, outline_r/2, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ONE); // center shaft
    screen.draw_centered_circle(center_x, center_y+p_led_placement_r, led_r, Pixel_state::PIXEL_ONE, 
        p_led_on ? Pixel_state::PIXEL_ONE:Pixel_state::PIXEL_ZERO); // p LED
    // For spread mode
    uint8_t delta;
    uint8_t min_range=0;
    uint8_t max_range=0;
    if (mode == Vpot_mode::SPREAD) {
        delta = abs(6 - value);
        min_range = 6-delta;
        max_range = 6+delta;
    }

    for (int angle_mult = -1; angle_mult <=9; angle_mult++) {
        float angle = (M_PI / 8.0) * static_cast<float>(angle_mult);
        float x0 = std::cos(angle) * led_placement_r;
        float y0 = std::sin(angle) * led_placement_r;
        uint8_t led_x = uint8_t((int)center_x - (int)x0);
        uint8_t led_y = uint8_t((int)center_y - (int)y0);
        Pixel_state state = Pixel_state::PIXEL_ZERO;
        uint8_t value_led = angle_mult + 2;
        
        if (value != 0) {
            switch (mode) {
                case Vpot_mode::SINGLE_DOT:
                    if (value_led == value)
                        state = Pixel_state::PIXEL_ONE;
                    break;
                case Vpot_mode::BOOST_CUT:
                    if (value == 6) {
                        if (value_led == value)
                            state = Pixel_state::PIXEL_ONE;
                    }
                    if (value > 6) {
                        // boost
                        if (value_led <= (value) && value_led >= 6)
                            state = Pixel_state::PIXEL_ONE;
                    }
                    else {
                        // cut
                        if (value_led >= value && value_led <= 6)
                            state = Pixel_state::PIXEL_ONE;
                    }
                    break;
                case Vpot_mode::WRAP:
                    if (value_led <= (value))
                        state = Pixel_state::PIXEL_ONE;
                    break;
                case Vpot_mode::SPREAD:
                    if (value == 6) {
                        if (value_led == value)
                            state = Pixel_state::PIXEL_ONE;
                    }
                    else {
                        if (value_led >= min_range && value_led <= max_range)
                            state = Pixel_state::PIXEL_ONE;
                    }
                    break;
            }
        }
        screen.draw_centered_circle(led_x, led_y, led_r, Pixel_state::PIXEL_ONE, state);
    }
}

void rppicomidi::Mc_vpot_display::set_by_cc_value(uint8_t cc_value)
{
    p_led_on = (cc_value & 0x40) != 0;
    mode = static_cast<Vpot_mode>((cc_value >>4) & 3);
    value = cc_value & 0xf;
    if (value > 11)
        value = 0;
    draw();
}
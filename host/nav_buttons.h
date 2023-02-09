/**
 * @file nav_buttons.h
 * @brief this class describes a driver for 7 nav buttons or a 5-way
 * "joystick" switch plus two nav buttons. It triggers View_manager
 * events on button presses.
 *
 * The button mapping to GPIO pins on the RP2040 is defined using
 * the BUTTON_* macros below by default unless they are defined
 * in the CMakeLists.txt file
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
// Button GP number default definitions; assumption is that the set of 7 buttons
// use a continuous group of 7 GPIO pins. The order of the pins and the first
// pin number is controlled by the following preprocessor defines
#ifndef BUTTON_FIRST_GPIO
#define BUTTON_FIRST_GPIO 6
#endif
#ifndef BUTTON_UP
#define BUTTON_UP 6
#endif
#ifndef BUTTON_DOWN
#define BUTTON_DOWN 7
#endif
#ifndef BUTTON_LEFT
#define BUTTON_LEFT 8
#endif
#ifndef BUTTON_RIGHT
#define BUTTON_RIGHT 9
#endif
#ifndef BUTTON_ENTER
#define BUTTON_ENTER 10
#endif
#ifndef BUTTON_BACK
#define BUTTON_BACK 11
#endif
#ifndef BUTTON_SHIFT
#define BUTTON_SHIFT 12
#endif

namespace rppicomidi {
class Nav_buttons
{
public:
    Nav_buttons();
    /**
     * @brief poll all 8 buttons and return changed status
     * 
     * @return 0 if unchanged since last poll or uint8_t bitmap as follows
     *   bit 7: Button status has changed
     *   bit 6: SHIFT button is pressed
     *   bit 5: BACK button is pressed
     *   bit 4: SELECT button is pressed
     *   bit 3: RIGHT button is pressed
     *   bit 2: LEFT button is pressed
     *   bit 1: DOWN button is pressed
     *   bit 0: UP button is pressed
     */
    uint8_t poll();
    const char* get_button_name(uint8_t button_map);
private:
    static const uint8_t ndebounce=10;
    uint8_t debounce[ndebounce];
    uint8_t prev_buttons;
    absolute_time_t previous_timestamp;
    int32_t held_buttons_timeout;
    const int32_t max_button_repeat_interval_ms;
    int32_t button_repeat_interval_ms;
    int32_t acceleration_count;
};
}
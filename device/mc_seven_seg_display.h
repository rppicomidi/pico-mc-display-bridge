/**
 * @file mc_seven_seg_display.h
 * @brief This class implements the Assignment 2-digit seven segment display
 * and the Timecode/BBT 10-digit seven segment display. Per digit decimal
 * point display is not supported
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
#include "view_manager.h"
#include "setup_menu.h"
namespace rppicomidi {
class Mc_seven_seg_display : public View
{
public:
    /**
     * @brief Construct a new Mc_seven_seg_display object
     * 
     * @param screen_ 
     * @param x_ 
     * @param y_ 
     * @param channel_ 
     */
    Mc_seven_seg_display(View_manager& view_manager_, Mono_graphics& screen_, bool smpte_led_, bool beats_led_, Setup_menu& setup_menu_);

    virtual ~Mc_seven_seg_display()=default;

    void draw() final;
    Select_result on_select() final;

    /**
     * @brief Set the text in one of the seven-segment display digits
     *
     * There are two seven segment displays: The 2-digit 7-segment display
     * Assignment display has rightmost digit 10 and the leftmost digit 11.
     *
     * The 10-digit Timecode/BBT display has the rightmost digit 0 and the
     * leftmost digit 9.
     * 
     * @param digit_ is the digit of the symbol
     * @param symbol_ is the ASCII encoding of the symbol
     *
     * @note decimal point display is not supported. Decimal point
     * and colon displays are determined by the Timecode/BBT display mode
     * for the 10-digit display.
     */
    void set_seven_seg_digit(uint8_t digit_, const char symbol_);

    /**
     * @brief Extract the digit text for the display digit from a Mackie Control 
     * seven segment display CC message
     *
     * @param byte1 is 0x4?, where ? is the digit number.
     * @param byte2 is 0x??, where ?? is the Mackie Control encoding for the
     * text on a seven-segment display.
     * @note the decimal point value is ignored
     */
    bool set_digit_by_mc_cc(uint8_t byte1, uint8_t byte2);

    bool set_digits_by_mc_sysex(uint8_t* sysex, int len_sysex);
    /**
     * @brief Set the states of the SMPTE LED and the BEATS LED
     *
     * @param smpte_led true means labels are for SMPTE Timecode
     * @param beats_led true means labels are for BEATS LED
     *
     * @note if both are true, or neither are true, no labels are displayed
     */
    void set_smpte_beats_leds(bool smpte_led, bool beats_led);

    /**
     * @brief Extract the state of the SMPTE LED or the BEATS LED from
     * a Mackie Control note message
     * 
     * @param byte1 0x71 is for SMPTE LED. 0x72 is for BEATS LED
     * @param byte2 0x00 is LED off, 0x01 or 0x7f is on
     */
    bool set_smpte_beats_by_mc_note(uint8_t byte1, uint8_t byte2);
private:
    // Get rid of default constructor and copy constructor
    Mc_seven_seg_display() = delete;
    Mc_seven_seg_display(Mc_seven_seg_display&) = delete;
    void center_label(const char* text, uint8_t big_chars_x, uint8_t big_chars_y, uint8_t nbig_chars, bool under);
    View_manager& view_manager;
    const Mono_mono_font& seven_seg_font;
    const Mono_mono_font& label_font;
    bool smpte_led;
    bool beats_led;
    const uint8_t nbeat_digits;
    const uint8_t nbars_digits;
    const uint8_t nsubs_digits;
    const uint8_t nticks_digits;
    const uint8_t nmode_digits;
    char digits[12];
    Setup_menu& setup_menu;
};
}
/**
 * @file mc_seven_seg_display.cpp
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
#include <cstring>
#include <cstdio>
#include "mc_seven_seg_display.h"

rppicomidi::Mc_seven_seg_display::Mc_seven_seg_display(View_manager& view_manager_, Mono_graphics& screen_, bool smpte_led_, bool beats_led_, Setup_menu& setup_menu_) :
    View{screen_, screen_.get_clip_rect()},
    view_manager{view_manager_}, seven_seg_font{screen.get_font_24()}, label_font{screen.get_font_8()},
    smpte_led{smpte_led_}, beats_led{beats_led_},
    nbeat_digits{3}, nbars_digits{2}, nsubs_digits{2}, nticks_digits{3}, nmode_digits{2},
    setup_menu{setup_menu_}
{
    memset(digits, ' ', sizeof(digits));
}
void rppicomidi::Mc_seven_seg_display::center_label(const char* text, uint8_t big_chars_x, uint8_t big_chars_y, uint8_t nbig_chars, bool under)
{
    if (view_manager.is_current_view(this)) {
        int x = big_chars_x + (seven_seg_font.width*nbig_chars)/2 - (strlen(text)*label_font.width)/2;
        int y = big_chars_y + (under ? seven_seg_font.height:-label_font.height);

        screen.draw_string(label_font,x,y,text, strlen(text),Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
    }
}

void rppicomidi::Mc_seven_seg_display::draw()
{
    if (view_manager.is_current_view(this)) {
        screen.clear_canvas();
        //uint8_t tc_x = 0;
        uint8_t tc_y = screen.get_screen_height() - seven_seg_font.height - label_font.height;
        // draw the timecode digit labels
        set_smpte_beats_leds(smpte_led, beats_led);
        // draw the timecode and mode digits
        for (size_t digit = 0; digit < sizeof(digits); digit++) {
            set_seven_seg_digit(digit, digits[digit]);
        }
        // draw the mode label
        center_label("mode",0,tc_y,nmode_digits,true);
    }
}

rppicomidi::Mc_seven_seg_display::Select_result rppicomidi::Mc_seven_seg_display::on_select(View**)
{
    printf("setup menu requested\r\n");
    view_manager.push_view(&setup_menu);
    return new_view; // the top of the view stack should remain unchanged
}

void rppicomidi::Mc_seven_seg_display::set_seven_seg_digit(uint8_t digit_, const char symbol_)
{
    if (digit_< 10) {
        // Timecode or Beats, Bars, Subdivisions and Ticks
        bool under = false;
        uint8_t tc_y = under?0:label_font.height;
        uint8_t tc_x;
        if (digit_ < 3) {
            // last 3 digits
            tc_x = seven_seg_font.width*(nbeat_digits+nbars_digits+nsubs_digits+2-digit_)+ 8;
        }
        else if (digit_ < 5) {
            tc_x = seven_seg_font.width*(nbeat_digits+nbars_digits+4-digit_)+ 5;
        }
        else if (digit_ < 7) {
            tc_x = seven_seg_font.width*(nbeat_digits+6-digit_)+ 2;
        }
        else {
            tc_x = seven_seg_font.width*(9-digit_);
        }
        digits[digit_] = symbol_;
        if (view_manager.is_current_view(this)) {
            screen.draw_character(seven_seg_font, tc_x, tc_y, symbol_, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
        }
    }
    else if (digit_ < 12) {
        bool under = true;
        uint8_t mode_y = screen.get_screen_height()- seven_seg_font.height + (under ? -label_font.height:0);
        uint8_t mode_x = seven_seg_font.width * (11-digit_);
        // Mode digit
        digits[digit_] = symbol_;
        if (view_manager.is_current_view(this)) {
            screen.draw_character(seven_seg_font, mode_x, mode_y, symbol_, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
        }
    }
}

void rppicomidi::Mc_seven_seg_display::set_smpte_beats_leds(bool smpte_led_, bool beats_led_)
{
    smpte_led = smpte_led_;
    beats_led = beats_led_;
    if (view_manager.is_current_view(this)) {
        bool under = false;
        uint8_t tc_y = under?0:label_font.height;
        uint8_t tc_x = 0;
        uint8_t label_y = tc_y + (under?seven_seg_font.height:-label_font.height);
        uint8_t punct_y = tc_y + seven_seg_font.height - 5;
        // clear the labels
        screen.draw_rectangle(tc_x, label_y, screen.get_screen_width(), label_font.height, Pixel_state::PIXEL_ZERO, Pixel_state::PIXEL_ZERO);
        if ((smpte_led && !beats_led) || (!smpte_led && beats_led)) {
            bool is_timecode_mode = smpte_led;
            // draw new labels
            const char* label = is_timecode_mode ? "hours" : "bars";
            center_label(label, tc_x, tc_y,nbeat_digits, false);
            tc_x += (seven_seg_font.width * nbeat_digits);
            screen.draw_rectangle(tc_x, punct_y, 2, 5, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ONE);
            tc_x += 2;
            label = is_timecode_mode ? "mins" : "beat";
            center_label(label, tc_x, tc_y,nbars_digits, false);
            tc_x += (seven_seg_font.width * nbars_digits);
            screen.draw_rectangle(tc_x, punct_y, 2, 5, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ONE);
            tc_x += 3;
            label = is_timecode_mode ? "secs" : "subs";
            center_label(label, tc_x, tc_y,nsubs_digits, false);
            tc_x += (seven_seg_font.width * nsubs_digits);
            screen.draw_rectangle(tc_x, punct_y, 2, 5, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ONE);
            tc_x += 3;
            label = is_timecode_mode ? "frames" : "ticks";
            center_label(label, tc_x, tc_y,nticks_digits, false);
        }
    }
}

bool rppicomidi::Mc_seven_seg_display::set_digit_by_mc_cc(uint8_t byte1, uint8_t byte2)
{
    bool success = true;
    if ((byte1 & 0xf0) == 0x40) {
        uint8_t digit = byte1 & 0xf;
        char symbol = ' ';
        byte2 &= 0x3f; //strip off the decimal point
        if (digit < 12) {
            if (byte2 < 0x20) {
                symbol = byte2 +'@';
                // some letters should be lower case to be consistent with the 7-segment font
                switch (symbol) {
                    case 'B':
                    case 'D':
                    case 'H':
                    case 'N':
                    case 'O':
                    case 'Q':
                    case 'R':
                    case 'T':
                    case 'U':
                        symbol += 0x20;
                        break;
                    default:
                        break;
                }
            }
            else {
                symbol = (byte2 - 0x20) + ' ';
            }
        }
        set_seven_seg_digit(digit, symbol);
    }
    else {
        success = false;
    }
    return success;
}

bool rppicomidi::Mc_seven_seg_display::set_digits_by_mc_sysex(uint8_t* sysex, int len_sysex)
{
    bool success = false;
    if (len_sysex > 7 && sysex[0]==0xf0 && sysex[1]==0x00 && sysex[2]==0x00 && sysex[3]==0x66 && (sysex[4]==0x14 || sysex[4]==0x10) && sysex[5]==0x10 && sysex[len_sysex-1]==0xf7) {
        success = true;
        uint8_t num_digits = len_sysex - 7;
        for (int digit=0; success && digit < num_digits; digit++) {
            if (!set_digit_by_mc_cc(0x40 | digit, sysex[6+digit]))
                success = false;
        }
    }
    return success;
}

bool rppicomidi::Mc_seven_seg_display::set_smpte_beats_by_mc_note(uint8_t byte1, uint8_t byte2)
{
    bool success = true;
    if (byte1 == 0x71) {
        smpte_led = (byte2 == 0x01 || byte2 == 0x7f);
    }
    else if (byte1 == 0x72) {
        beats_led = (byte2 == 0x01 || byte2 == 0x7f);
    }
    else {
        success = false;
    }
    if (success) {
        set_smpte_beats_leds(smpte_led, beats_led);
    }
    return success;
}
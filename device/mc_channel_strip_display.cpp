/**
 * @file mc_channel_strip_display.cpp
 * @brief This class manages a single channel strip display that shows
 * two lines of seven characters, the current VPot LED status, the
 * meter status, and status of the channel's REC, SOLO, MUTE and SEL
 * button LEDs. The class conforms to the Mackie Control protocol.
 * This class requires the screen dimensions to be 64x128 so that
 * the layout looks correct
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
#include "mc_channel_strip_display.h"
#include "mc_channel_text.h"
#include "mc_meter.h"
#include "mc_vpot_display.h"
#include "text_box.h"
//#include "ext_lib/ssd1306/src/driver_ssd1306_font.h"
//#include "ext_lib/RPi-Pico-SSD1306-library/font.hpp"

rppicomidi::Mc_channel_strip_display::Mc_channel_strip_display(Mono_graphics& screen_, uint8_t channel_) :
    screen{screen_}, channel{channel_},
    channel_text{screen,4,94,channel,screen.get_font_16()},
    meter{screen, 56, 0, channel_},
    vpot_display{screen, 0, 52, Vpot_mode::BOOST_CUT, 0, false},
    rec{screen, 0, 0 ,28, 12, "Rec", screen.get_font_8(), false},
    mute{screen, 0, 12 ,28, 12, "Mute", screen.get_font_8(), false},
    solo{screen, 0, 24 ,28, 12, "Solo", screen.get_font_8(), false},
    sel{screen, 0, 36 ,28, 12, "Sel", screen.get_font_8(), false}
{
    assert(screen.get_screen_height()==128 && screen.get_screen_width()==64);
    disp_objects.push_back(&channel_text);
    disp_objects.push_back(&meter);
    disp_objects.push_back(&vpot_display);
    disp_objects.push_back(&rec);
    disp_objects.push_back(&mute);
    disp_objects.push_back(&solo);
    disp_objects.push_back(&sel);
    draw();
}

void rppicomidi::Mc_channel_strip_display::draw()
{
    for (auto& it : disp_objects) {
        it->draw();
    }    
}

bool rppicomidi::Mc_channel_strip_display::push_midi_message(uint8_t* message, int nbytes)
{
    bool success = true;
    switch (message[0]) {
        case 0x90:
            // note message
            if (message[1] == channel) {
                // rec status
                rec.set_state(message[2] != 0);
            }
            else if (message[1] == channel + 0x8) {
                // solo status
                solo.set_state(message[2] != 0);
            }
            else if (message[1] == channel + 0x10) {
                // mute status
                mute.set_state(message[2] != 0);
            }
            else if (message[1] == channel +0x18) {
                // select status
                sel.set_state(message[2] != 0);
            }
            else {
                success = false;
            }
            break;
        case 0xB0:
            // CC message. Might be VPot status
            if ((message[1] & 0xf0) == 0x30 && (message[1] & 0xf) == channel) {
                vpot_display.set_by_cc_value(message[2]);
            }
            else {
                success = false;
            }
            break;
        case 0xD0:
            if (((message[1] >>4 )&0x7) == channel) {
                meter.set_value_by_channel_pressure(message[1]);
            }
            else {
                success = false;
            }
            break;
        case 0xF0:
            // Might be set channel text message. Must start with F0 00 00 66 14|15 12 oo and end with F7 so
            // must be greater than 8 bytes long or else it contains no character data and will have no effect
            if (nbytes>8 && message[1]==0x00 && message[2]==0x00 && message[3]==0x66 && (message[4]==0x14 || message[4]==0x15) && message[5]==0x12 &&
                message[nbytes-1] == 0xf7) {
                success = channel_text.set_text_by_mc_sysex(message+6, nbytes - 8);
            }
            break;
        default:
            success = false;
            break;
    }
    return success;
}

void rppicomidi::Mc_channel_strip_display::task()
{
    meter.task();
}

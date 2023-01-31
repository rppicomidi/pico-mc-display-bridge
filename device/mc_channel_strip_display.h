/**
 * @file mc_channel_strip_display.h
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

#pragma once
#include "drawable.h"
#include "mono_graphics_lib.h"
#include "mc_channel_strip_display.h"
#include "mc_channel_text.h"
#include "mc_meter.h"
#include "mc_vpot_display.h"
#include "text_box.h"
#include <vector>
namespace rppicomidi {
class Mc_channel_strip_display : Drawable 
{
public:
    Mc_channel_strip_display(Mono_graphics& screen_, uint8_t channel);
    virtual ~Mc_channel_strip_display() = default;

    void draw();

    /**
     * @brief run any tasks of the screen components and process any complete
     * messages in the midi stream ring buffer.
     */
    void task();

    /**
     * @brief send a MIDI message for possible processing
     * 
     * @param stream a pointer to the bytes to add
     * @param nbytes the number of bytes to add
     * @return true if message is consumed, false otherwise
     */
    bool push_midi_message(uint8_t* message, int nbytes);
private:
    Mc_channel_strip_display() = delete;
    Mc_channel_strip_display(Mc_channel_strip_display&) = delete;

    Mono_graphics& screen;
    uint8_t channel;

    Mc_channel_text channel_text;
    Mc_meter meter;
    Mc_vpot_display vpot_display;
    Text_box rec;
    Text_box mute;
    Text_box solo;
    Text_box sel;
    std::vector<Drawable*> disp_objects;
};
}

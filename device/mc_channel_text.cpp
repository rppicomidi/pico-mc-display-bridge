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
#include <cstring>
#include "mc_channel_text.h"
#include "pico/assert.h"
rppicomidi::Mc_channel_text::Mc_channel_text(Mono_graphics& screen_, uint8_t x_, uint8_t y_, uint8_t channel_, const Mono_mono_font& font_) :
    screen{screen_}, x{x_}, y{y_}, channel{channel_}, font{font_}
{
    // pad with ' '  1234567
    strcpy(text[0], "       ");
    strcpy(text[1], "Ch     ");
    text[1][3] = channel+'1';
    draw();
}

void rppicomidi::Mc_channel_text::draw()
{
    screen.draw_line(0, y, screen.get_screen_width()-1, y, Pixel_state::PIXEL_ONE, true);
    for (int idx = 0; idx < 2; idx++) {
        screen.draw_string(font, x, 2+y + idx* font.height, text[idx], 7, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
    }
}

void rppicomidi::Mc_channel_text::set_text(uint8_t line, uint8_t offset, const char* text_)
{
    assert(line < 2);
    assert(offset < 7);
    // non-destructive (don't add null termination) strncpy
    for (int idx=offset; idx < 7 && text_[idx] != '\0'; idx++) {
        // If the character is not in the font character set, display
        // the first character in the font instead (usually space)
        char ch = text_[idx];
        if (ch<font.first_char && ch>font.last_char)
            ch = font.first_char;
        text[line][idx] =  ch;
    }
    draw();
}

bool rppicomidi::Mc_channel_text::set_text_by_mc_sysex(const uint8_t* sysex_message, uint8_t num_chars)
{
    assert(num_chars >= 1);
    assert(sysex_message);
    bool success = false;
    uint8_t offset= sysex_message[0];
    uint8_t line_offset = channel *7;
    for (int line = 0; line < 2; line++) {
        if ((line_offset+6) >= offset) {
            // then there may be characters for this field
            int field_offset = offset %7;
            int char_offset = (line_offset > offset) ? 0: field_offset;
            uint8_t message_char_idx = line_offset + char_offset - offset + 1;
            for (int ch_idx = char_offset; ch_idx < 7 && message_char_idx <= num_chars; ch_idx++) {
                text[line][ch_idx] = sysex_message[message_char_idx++];
                success = true;
            }
        }
        line_offset+=56;
    }
    if (success) {
        draw();
    }
    return success;
}
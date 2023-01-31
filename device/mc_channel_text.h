/**
 * @file mc_channel_text.h
 * @brief This class implements two lines of 7 characters for a Mackie
 * Control protocol channel strip display.
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
class Mc_channel_text : public Drawable
{
public:
    /**
     * @brief Construct a new Mc_channel_text object
     * 
     * @param screen_ 
     * @param x_ 
     * @param y_ 
     * @param channel_ 
     */
    Mc_channel_text(Mono_graphics& screen_, uint8_t x_, uint8_t y_, uint8_t channel_, const Mono_mono_font& font_);

    virtual ~Mc_channel_text()=default;

    void draw() final;

    /**
     * @brief Set the text in one of the two lines to be displayed
     * 
     * @param line line number either 0 or 1; others crash
     * @param offset is the number of characters to skip before drawing
     * @param text a null-terminated string up to 7-offset characters long.
     *
     * @note if offset is not 0, no spaces will be inserted on the left.
     * @note if the text is shorter than 7-offset bytes long, no spaces
     * will be inserted to the right
     * 
     */
    void set_text(uint8_t line, uint8_t offset, const char* text_);

    /**
     * @brief Extract the text for this channel from a Mackie Control 
     * LCD system exclusive message
     * 
     * The Mackie Control LCD assumes a single two line by 56 character
     * display. The LCD message format is
     * 0xF0 0x00 0x00 0x66 0x14 0x12 oo [up to 112 characters] 0xF7 where
     * oo is display line offset 0-55 for the first line and and 56-111
     * for the second line.
     * 
     * @param sysex_message A pointer to the sysex message body starting at the oo byte
     * @param num_chars the number of characters to write (excludes the oo byte)
     */
    bool set_text_by_mc_sysex(const uint8_t* sysex_message, uint8_t num_chars);
private:
    // Get rid of default constructor and copy constructor
    Mc_channel_text() = delete;
    Mc_channel_text(Mc_channel_text&) = delete;

    Mono_graphics& screen;
    uint8_t x,y;
    uint8_t channel;
    char text[2][8]; // An array of 2 7-character null-terminated strings always right padded with spaces
    const Mono_mono_font& font;
};
}
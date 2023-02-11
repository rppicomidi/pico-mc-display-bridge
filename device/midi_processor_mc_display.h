/**
 * MIT License
 *
 * Copyright (c) 2023 rppicomidi
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
#include "midi_processor.h"
#include "midi_processor_mc_display_core.h"
namespace rppicomidi
{
class Midi_processor_mc_display : public Midi_processor
{
public:
    Midi_processor_mc_display() = delete;
    Midi_processor_mc_display(uint16_t unique_id_) : Midi_processor{static_getname(), unique_id_} {}
    bool process(uint8_t* packet) final { return Midi_processor_mc_display_core::instance().process(packet); }
    static const char* static_getname() { return "MC Display"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) {return new Midi_processor_mc_display(unique_id_); }
private:

};
}
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
#include "mc_seven_seg_display.h"
#include "mc_channel_strip_display.h"
namespace rppicomidi
{
class Midi_processor_mc_display_core
{
public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Midi_processor_mc_display_core& instance()
    {
        static Midi_processor_mc_display_core _instance;    // Guaranteed to be destroyed.
                                                    // Instantiated on first use.
        return _instance;
    }
    Midi_processor_mc_display_core(Midi_processor_mc_display_core const&) = delete;
    void operator=(Midi_processor_mc_display_core const&) = delete;

    /**
     * @brief initialize the key variables of the instance
     * 
     * @note Recommended to call this function to create the instance:
     *       Midi_processor_mc_display_core::instance().init();
     * @param num_chan_displays_ usually 8
     * @param channel_disp_ an array of num_chan_displays_ Mc_channel_strip_display View pointers
     * @param seven_seg_ a pointer to the 7-segment display View
     */
    void init(uint8_t num_chan_displays_, Mc_channel_strip_display **channel_disp_, Mc_seven_seg_display* seven_seg_) {
        num_chan_displays = num_chan_displays_;
        channel_disp = channel_disp_;
        seven_seg = seven_seg_;
    }
    bool process(uint8_t* packet);
    void create_serial_number();

    void set_cable(uint8_t cable_) { cable_num = cable_; if (cable_cb) cable_cb(cable_, set_cable_context); }
    void register_set_cable_callback(void (*cable_cb_)(uint8_t, void*), void* context_) {cable_cb =cable_cb_; set_cable_context = context_; }
private:
    Midi_processor_mc_display_core() :
            num_chan_displays{0}, channel_disp{nullptr}, seven_seg{nullptr}, sysex_idx{0}, waiting_for_eox{false}, cable_cb{nullptr}, set_cable_context{nullptr}
    {
        create_serial_number(); // make sure the serial_number variable is valid;
    }
    bool handle_mc_device_inquiry();
    uint8_t num_chan_displays;
    Mc_channel_strip_display **channel_disp;
    Mc_seven_seg_display* seven_seg;
    static const size_t max_sysex = 2048; // the maximum MC SySex message is 120 bytes long, but fortune favors the prepared
    uint8_t sysex_message[max_sysex];
    size_t sysex_idx;  // the index into the sysex_message array
    bool waiting_for_eox;
    uint8_t serial_number[7];
    void (*cable_cb)(uint8_t, void*);
    void* set_cable_context;
    uint8_t cable_num;
};
}
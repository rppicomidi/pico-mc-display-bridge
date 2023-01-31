/**
 * @file mc_fader_sync.cpp
 * @brief This class is used for managing fader soft pickup synchronization
 * for Mackie Control Protocol compatible non-motorized faders
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

#include "mc_fader_sync.h"
//#include <cstdio>
rppicomidi::Mc_fader_sync::Mc_fader_sync()
{
    start_sync();
}

void rppicomidi::Mc_fader_sync::start_sync()
{
    state = Sync_state::Unknown;
    fader_position = impossible_fader_value;
    target_position = impossible_fader_value;
}

rppicomidi::Mc_fader_sync::Sync_state rppicomidi::Mc_fader_sync::update_fader_position(uint8_t byte1, uint8_t byte2)
{
    fader_position = pitch_bend_to_position(byte1, byte2);
    //printf("update_fader_position: byte1=%02x byte2=%02x pos=%04x\r\n", byte1, byte2, fader_position);
    if (target_position == impossible_fader_value) {
        state = Sync_state::Unknown;
    }
    else if (state == Sync_state::Unknown) {
        if (target_position > fader_position) {
            state = Sync_state::Increase;
        }
        else if (target_position < fader_position) {
            state = Sync_state::Decrease;
        }
        else {
            state = Sync_state::Synchronized;
        }
    }
    else if (state == Sync_state::Increase) {
        if (fader_position >= target_position) {
            state = Sync_state::Synchronized;
        }
    }
    else if (state == Sync_state::Decrease) {
        if (fader_position <= target_position) {
            state = Sync_state::Synchronized;
        }
    }
    // If state is synchronized, do nothing.
    return state;
}

rppicomidi::Mc_fader_sync::Sync_state rppicomidi::Mc_fader_sync::update_target_position(uint8_t byte1, uint8_t byte2)
{
    target_position = pitch_bend_to_position(byte1, byte2);
    if (fader_position == impossible_fader_value) {
        state = Sync_state::Unknown;
    }
    else if (fader_position == target_position) {
        state = Sync_state::Synchronized;
    }
    else {
        // The target position moved. Wait for new fader position to synchronize
        fader_position = impossible_fader_value;
        state = Sync_state::Unknown;
    }
    return state;
}

const char* rppicomidi::Mc_fader_sync::get_state_string(rppicomidi::Mc_fader_sync::Sync_state state)
{
    const char* ret ="Unknown";
    switch (state) {
        case Sync_state::Decrease:
            ret = "Decrease";
            break;
        case Sync_state::Increase:
            ret = "Increase";
            break;
        case Sync_state::Synchronized:
            ret = "Synchronized";
            break;
        default:
            break;
    }
    return ret;
}
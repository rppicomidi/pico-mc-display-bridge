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
#include "midi_processor_mc_display_core.h"

#include "tusb.h"
bool rppicomidi::Midi_processor_mc_display_core::process(uint8_t* rx)
{
    uint8_t nread = 0;
    uint8_t const code_index = rx[0] & 0x0f;
    bool pass_it_on = false;
    // MIDI 1.0 Table 4-1: Code Index Number Classifications
    switch(code_index)
    {
        case MIDI_CIN_MISC:
        case MIDI_CIN_CABLE_EVENT:
            // These are reserved and unused, possibly issue somewhere, skip this packet
            break;

        case MIDI_CIN_SYSEX_END_1BYTE:
        case MIDI_CIN_1BYTE_DATA:
            nread = 1;
            break;

        case MIDI_CIN_SYSCOM_2BYTE     :
        case MIDI_CIN_SYSEX_END_2BYTE  :
        case MIDI_CIN_PROGRAM_CHANGE   :
        case MIDI_CIN_CHANNEL_PRESSURE :
            nread = 2;
            break;

        default:
            nread = 3;
            break;
    }
    bool success = false;
    int jdx;
    int dropped_sysex = 0;
    switch(rx[1]) {
        case 0x90: // Note on (LED control message)
            // Note message, channel 1
            success = seven_seg->set_smpte_beats_by_mc_note(rx[2], rx[3]);
            if (!success) {
                for (int chan = 0; !success && chan < num_chan_displays; chan++) {
                    success = channel_disp[chan]->push_midi_message(rx+1, nread);
                }
            }
            pass_it_on = true;
            break;
        case 0xB0: // Logic Control 7-segment message or VPot message
            // CC message
            success = seven_seg->set_digit_by_mc_cc(rx[2], rx[3]);
            // filter this out if is a 7-seg LED digit message
            pass_it_on = !success;
            if (!success) {
                for (int chan = 0; !success && chan < num_chan_displays; chan++) {
                    success = channel_disp[chan]->push_midi_message(rx +1, 3);
                }
            }
            // only push the message if not a display message
            if (success)
                pass_it_on = false;
            break;
        case 0xBF: // 7-segment display message alternate
            // Alternate CC message for timecode.
            success = seven_seg->set_digit_by_mc_cc(rx[2], rx[3]);
            // only push the message if not a display message
            pass_it_on = !success;
            break;
        case 0xD0:  // Channel pressure (meter message)
            for (int chan = 0; !success && chan < num_chan_displays; chan++) {
                success = channel_disp[chan]->push_midi_message(rx +1, 2);
            }
            // only push the message if not a display message
            pass_it_on = !success;
            break;
        case 0xF0: // sysex start
            sysex_message[0] = rx[1];
            sysex_idx = 1;
            dropped_sysex = 0;
            for (jdx = 2; jdx < 4 && rx[jdx] != 0xF7; jdx++) {
                sysex_message[sysex_idx] = rx[jdx];
                // truncate sysex messages longer than we can store but leave room for EOX
                if (sysex_idx < max_sysex-1) {
                    ++sysex_idx;
                }
                else {
                    ++dropped_sysex;
                }
            }
            if (jdx == 4) {
                waiting_for_eox = true;
            }
            else {
                // must have found eox
                sysex_message[sysex_idx] = rx[jdx]; // copy eox
                if (dropped_sysex != 0) {
                    TU_LOG1("Warning dropped %d sysex bytes\r\n", dropped_sysex);
                }
                if (!handle_mc_device_inquiry()) {
                    if (!seven_seg->set_digits_by_mc_sysex(sysex_message, sysex_idx+1)) {
                        int nsuccessful = 0;
                        // have to try every channel strip
                        for (int chan = 0; chan < num_chan_displays; chan++) {
                            if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                ++nsuccessful;
                            }
                        }
                        success = nsuccessful != 0;
                    }
                    if (!success) {
                        // send it on over the MIDI UART
                        pass_it_on = true;;

                        printf("unhandled:\r\n");
                        for (size_t kdx=0; kdx<=sysex_idx;kdx++) {
                            printf("%02x ", sysex_message[kdx]);
                        }
                        printf("\n\r");
                    }
                }
                waiting_for_eox = false;
                sysex_idx = 0;
            }
            break;
        case 0xF7: // EOX
            if (waiting_for_eox) {
                if (dropped_sysex != 0) {
                    TU_LOG1("Warning dropped %d sysex bytes\r\n", dropped_sysex);
                }

                sysex_message[sysex_idx] = rx[1];
                waiting_for_eox = false;
                if (!handle_mc_device_inquiry()) {
                    if (!seven_seg->set_digits_by_mc_sysex(sysex_message, sysex_idx+1)) {
                        // have to try every channel strip
                        int nsuccessful = 0;
                        for (int chan = 0; chan < num_chan_displays; chan++) {
                            if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                ++nsuccessful;
                            }
                        }
                        success = nsuccessful != 0;
                    }
                    if (!success) {
                        pass_it_on = true; // send it on to the MIDI UART
                        printf("unhandled:\r\n");
                        for (size_t kdx=0; kdx<=sysex_idx;kdx++) {
                            printf("%02x ",sysex_message[kdx]);
                        }
                        printf("\n\r");
                    }
                }
            }
            waiting_for_eox = false;
            sysex_idx = 0;
            break;
        default:
            if (waiting_for_eox) {
                // Might be the middle of a sysex message or the tail end of one
                for (jdx = 1; jdx < 4 && (rx[jdx] & 0x80) == 0; jdx++) {
                    sysex_message[sysex_idx++] = rx[jdx];
                }
                if (jdx < 4) {
                    if (rx[jdx] == 0xF7) {
                        // we have the sysex whole message message
                        if (dropped_sysex != 0) {
                            TU_LOG1("Warning dropped %d sysex bytes\r\n", dropped_sysex);
                        }

                        sysex_message[sysex_idx] = 0xF7;
                        waiting_for_eox = false;
                        if (!handle_mc_device_inquiry()) {
                            if (!seven_seg->set_digits_by_mc_sysex(sysex_message, sysex_idx+1)) {
                                // have to try every channel strip
                                int nsuccessful = 0;
                                for (int chan = 0; chan < num_chan_displays; chan++) {
                                    if (channel_disp[chan]->push_midi_message(sysex_message, sysex_idx+1)) {
                                        ++nsuccessful;
                                    }
                                }
                                success = nsuccessful != 0;
                            }
                            if (!success) {
                                pass_it_on = true; // send it on to the MIDI UART
                                printf("unhandled:\r\n");
                                for (size_t kdx=0; kdx<=sysex_idx;kdx++) {
                                    printf("%02x ",sysex_message[kdx]);
                                }
                                printf("\n\r");
                            }
                        }
                    }
                    else
                    {
                        // Something is very wrong. USB doesn't support inserting messages mid-stream
                    }
                    waiting_for_eox = false;
                    sysex_idx = 0;
                }
            }
            else {
                // Just an unhandled message. Pass it on for the host
                pass_it_on = true;
            }
            break;
    }
    return pass_it_on;
}

bool rppicomidi::Midi_processor_mc_display_core::handle_mc_device_inquiry()
{
    int nread = sysex_idx+1;
    bool handled = false;
    if (nread==7 && sysex_message[1]==0x00 && sysex_message[2]==0x00 && sysex_message[3]==0x66 &&
        sysex_message[4]==0x14 && sysex_message[5]==0x00 && sysex_message[6] == 0xf7) {
        // MC device query message; respond with the MC Host Connection Query
        uint8_t host_connection_query[]= { 0xF0, // start sysex
            0x00, 0x00, 0x66, // Mackie Manufacturer ID
            0x14, // MCU Pro (0x14) with all displays
            0x01, // Host Connection Query message ID
            serial_number[0], serial_number[1], serial_number[2], serial_number[3], serial_number[4], serial_number[5], serial_number[6], // arbitrary serial number
            0x01, 0x02, 0x03, 0x04, // arbitray
            0xf7, // EOX
        };
        uint32_t nwritten = tud_midi_stream_write(0, host_connection_query, sizeof(host_connection_query));
        if (nwritten != sizeof(host_connection_query)) {
            TU_LOG1("Warning: Dropped %lu bytes of host_connection_query message\r\n", sizeof(host_connection_query) - nwritten);
        }
        else {
            TU_LOG1("Sent Host Connection Query\r\n");
        }
        handled = true;
    }
    else if (nread==8 && sysex_message[1]==0x00 && sysex_message[2]==0x00 && sysex_message[3]==0x66 &&
        sysex_message[4]==0x14 && sysex_message[5]==0x1A && sysex_message[6] == 0x00 && sysex_message[7] == 0xf7) {
        uint8_t serial_number_response[] = { 0xF0, // start sysex
            0x00, 0x00, 0x66, // Mackie Manufacturer ID
            0x14, // MCU Pro
            0x1B, 0x58, 0x59, 0x5A, // Response to serial number request
            serial_number[0], serial_number[1], serial_number[2], serial_number[3], // The serial number
            0xF7 // EOX
        };
        uint32_t nwritten = tud_midi_stream_write(0, serial_number_response, sizeof(serial_number_response));
        if (nwritten != sizeof(serial_number_response)) {
            TU_LOG1("Warning: Dropped %lu bytes of serial_number_response message\r\n", sizeof(serial_number_response) - nwritten);
        }
        else {
            TU_LOG1("Sent Serial Number Response\r\n");
        }
        handled = true;
    }
    else if (nread >= 6 && sysex_message[1]==0x00 && sysex_message[2]==0x00 && sysex_message[3]==0x66 && sysex_message[4]==0x15) {
        // message is for an extender. Ignore it
        handled = true;
    }
    return handled;
}

void rppicomidi::Midi_processor_mc_display_core::create_serial_number()
{
    uint64_t now = time_us_64();
    serial_number[0] = now & 0x7Full;
    serial_number[1] = (now >> 8ull) & 0x7Full;
    serial_number[2] = (now >> 16ull) & 0x7Full;
    serial_number[3] = (now >> 24ull) & 0x7Full;
    serial_number[4] = (now >> 32ull) & 0x7Full;
    serial_number[5] = (now >> 40ull) & 0x7Full;
    serial_number[6] = (now >> 48ull) & 0x7Full;
}
/**
 * @file mc_fader_sync.h
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
#include <cstdint>
namespace rppicomidi {
class Mc_fader_sync
{
public:
    enum Sync_state {
        Unknown,        // Synchronization state unknown. Waiting for info from DAW, fader, or both
        Synchronized,   // target and fader are synchronized
        Increase,       // fader value needs to increase to synchronize fader and DAW
        Decrease,       // fader value needs to decrease to synchronize fader and DAW
    };
    static const int16_t max_fader_value = 16383; // Mackie Control fader values are 14 bits, so max fader value is 16383
    static const int16_t impossible_fader_value = max_fader_value+1; 
    Mc_fader_sync();
    /**
     * @brief Set the state to Unknown and the target and
     * fader position to impossible_fader value
     */
    void start_sync();

    /**
     * @brief Update the fader_position value based on the two
     * data bytes from a pitch bend message
     *
     * @param byte1 the first byte of the pitch bend message
     * @param byte2 the second byte of the pitch bend message
     * @return Sync_state the resulting synchronization state
     */
    Sync_state update_fader_position(uint8_t byte1, uint8_t byte2);

    /**
     * @brief Update the target_position value based on the two
     * data bytes from a pitch bend message
     *
     * @param byte1 the first byte of the pitch bend message
     * @param byte2 the second byte of the pitch bend message
     * @return Sync_state the resulting synchronization state
     */
    Sync_state update_target_position(uint8_t byte1, uint8_t byte2);

    /**
     * @brief Get the target position value
     * 
     * @return the target_position value
     */
    int16_t get_target_position() const {return target_position; }

    /**
     * @brief Get the fader position value
     * 
     * @return the fader_position value
     */
    int16_t get_fader_position() const {return fader_position; }

    static const char* get_state_string(Sync_state state);
private:
    // Return the position value base based on the data bytes of a pitch bend message
    int16_t pitch_bend_to_position(uint8_t byte1, uint8_t byte2) {return (byte2 << 7) | byte1; }
    int16_t target_position; // the value the DAW or other host software says should be the fader position
    int16_t fader_position;  // the fader position as reported by the fader on the control surface
    Sync_state state; // the current synchronization state
};
}
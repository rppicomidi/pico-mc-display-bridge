# pico-mc-display-bridge

Most Digital Audio Workstation (DAW) programs and many live audio products support Mackie Control (MC) protocol for controlling levels, transport controls, channel muting, etc. There are quite a few low-cost control surfaces on the market, but many lack features that more expensive controllers such as the Mackie MCU Pro or Behringer X-Touch support.

This project uses two Raspberry Pi Pico boards to implement a channel strip, meter, and time display for USB MIDI control surfaces that support Mackie Control (MC) protocol but do not have displays. You insert the pico-mc-bridge in line with the USB connection from the DAW to the control surface. One Pico board acts as a USB Device and connects to the DAW or other device that expects to talk with a MC control surface. The other Pico board acts as a USB host and connects to the MC compatible control surface, and it acts as a button interface for the additional buttons provided.

The two Pico boards together form what should be a transparent interface between the control surface and the DAW. The Host Pico reads the control surface USB device information and sends it to the Device Pico before the Device Pico initializes its USB stack. When the DAW computer asks the Device Pico for its device information, it sends the information of the connected control surface instead.

The pico-mc-display-bridge supports:

- one 128x64 OLED per channel strip that shows
	- 2 lines of 7 characters of text
	- a representation of the MC VPot LEDs,
	- a representation of the MC meter LEDs, 
	- a representation of the REC, MUTE, SOLO and SEL button LEDs.
- one 128x64 OLED for time display (either Bars/Beats/Subdivisions/ticks or SMPTE timecode), VPot mode display (a two-digit 7-segment display on the Mackie MCU Pro, for example). This display is shared with the MIDI Processor UI (see below)
- one button per channel strip that can function as either Select, Mute, Solo, Record
or VPot press depending on whether any of mode buttons is pressed.
- one button for choosing name or value display
- one button for choosing the time display mode (Bars/Beats/Subdivisions/Ticks or SMPTE Timecode).
- 7 buttons (or a 5-way navigation switch plus 2 buttons) for navigating the MIDI
processor UI
- a MIDI processor unit that determines which virtual MIDI cable in
the MIDI USB stream is carrying MC data, and that implements useful features
such as fader soft pickup to prevent value jumps when you move faders, button remap,
etc. The MIDI processor settings are stored in the Device Pico's program flash memory.
Each MC device type (unique USB VID and PID) can have up to 8 preset settings that you
can recall using the MIDI Processor UI.

# Source Organization and Build Instructions
## Source Organization

```
pico-mc-display-bridge/
    device/
    ext_lib/
    host/
    lib/
    LICENSE
    README.md    
```
The `device` directory contains the source code for the Device Pico and the `host` directory contains the source code for the Host Pico. You must build one image for each Pico, and program each Pico with the appropriate image.

## Get the Source Code

```
cd [the root directory where you want to put this project]
git clone --recurse-submodules https://github.com/rppicomidi/pico-mc-display-bridge.git
cd pico-mc-display-bridge
```

# Make sure you have the latest Pico C SDK
cd pico-sdk
git pull

# Get my fork of the tinyusb library with MIDI Host and device descriptor cloning support

```
cd lib/tinyusb
git remote add upstream https://github.com/hathach/tinyusb.git
git remote set-url origin https://github.com/rppicomidi/tinyusb.git
git fetch origin
git checkout -b pio-midihost origin/pio-midihost
```
# get the Pico-PIO-USB submodule into the source tree

```
cd hw/mcu/raspberry_pi
git submodule update --init Pico-PIO-USB
```

## Build the Device Pico Image
Assumes you just completed the "Get the Source Code" steps above. Also assumes you have correctly set up your command line build environment per the instructions in the _Getting started with Raspberry Pi Pico_ guide.

```
cd device
mkdir build
cd build
cmake ..
make
```

The build image is called `pico-mc-display-bridge-dev.uf2`. Use the one of the methods described in the _Getting started with Raspberry Pi Pico_ guide to program the Device Pico with this image.

## Build the Host Pico Image
Assumes you just completed the "Build the Device Pico Image" steps above

```
cd ../../host
mkdir build
cd build
cmake ..
make
```

The build image is called `pico-mc-display-bridge-host.uf2`. Use the one of the methods described in the _Getting started with Raspberry Pi Pico_ guide to program the Device Pico with this image.


# Pico-Pico Message Format
The two Pico boards are connected via UART1. If the interface only had to convey a single MIDI cable's worth of MIDI data, then the connection could contain the raw MIDI stream in both directions. However, the connected control surface may support up to 16 MIDI streams. In addition, the Device Pico needs to know some USB descriptor parameters from the MIDI device connected to the Host Pico:

- Vendor ID
- Product ID
- Number of virtual MIDI cables supported in each direction
- Product Name String, if available
- Vendor Name String, if available
- Serial Number String, if available
- Name of each virtual cable, if such information is available

All messages start with an header byte and are followed by an appropriate amount of data bytes, and then a checksum, which is the XOR of the header byte and all data bytes. Data values are in hex unless otherwise stated.

All messages that are not MIDI data stream messages are formatted:

```
cmd length [payload] checksum
```

where `cmd` is a single unsigned byte value larger than the largest MIDI header byte value, `length` is the number of bytes in the payload (may be 0), `payload` is the array of `length` data bytes (will be empty if `length` is 0) and `checksum` is the XOR of all bytes from the `cmd` byte to the last `payload` byte.

## MIDI data stream

```
header_byte midi_stream checksum
```

where `header_byte[7:4]` encodes the number of MIDI stream bytes that follow (1, 2 or 3, or 0 for a complete SysEx message).

`header_byte[3:0]` is the virtual cable number 0-F.

`midi_stream` is a stream of 1, 2 or 3 bytes, or a complete SysEx message that starts with `F0` and ends with `F7`.

Both Host Pico and Device Pico send this message to send bytes from the MIDI stream to the other Pico.

## Synchronize

```
48 checksum
```
The Device Pico will send this to the Host Pico when it is ready to receive MIDI data

## Request Device Descriptor

```
40 0 checksum
```
The Device Pico will request the 18-byte USB standard device descriptor for the connected device as soon as it is ready to receive the descriptor. The Host Pico will ignore this request until it has fully enumerated the connected device.

## Return Device Descriptor

```
41 12 [18 bytes of device descriptor] checksum
```
The Host Pico will return the 18-byte USB standard device descriptor for the connected device after the Pico Host has fully enumerated the connected device and after the Pico Host has received the Request Device Descriptor message.

## Request Configuration Descriptor block 0

```
49 0 checksum
```
The Device Pico will request the first block of configuration descriptor bytes from the Host Pico after the Device Pico has received the Return Device Descriptor message.

## Return Configuration Descriptor block 0

```
4A length [wTotalLength bytes of configuration descriptor or first block of bytes] checksum
```
The Host Pico will send the first block of configuration descriptor bytes to the Device Pico in response to the Request Configuration Descriptor block 0 command. This block of bytes
will contain the wTotalLength field, so the Device Pico will know if it needs to request
another block.

## Request Configuration Descriptor block 1

```
4B 0 checksum
```
The Device Pico will request the second block of configuration descriptor bytes from the Host Pico after the Device Pico has received the Return Configuration Descriptor block 0 message
only if there are more bytes in the configuration descriptor than can fit in a single block.

### Return Configuration Descriptor block 1

```
4C length [second block of bytes] checksum
```

## Request Configuration Descriptor block 2

```
4D 0 checksum
```
The Device Pico will request the third block of configuration descriptor bytes from the Host Pico after the Device Pico has received the Return Configuration Descriptor block 0 message
only if there are more bytes in the configuration descriptor than can fit in a the first two blocks.

## Return Configuration Descriptor block 2

```
4E length [second block of bytes] checksum
```

## Request Device String Indices

```
42 0 checksum
```
The Device Pico will send this message to the Host Pico in order to get a list of all string indices in the connected device. The Device Pico will send this after it receives the Core Device Info message.

## Return Device String Indexes

```
43 length [a list of length single byte string indices] checksum
```
The Host Pico sends this message. It contains a list of every string index the device
supports.

## Request LangIDs (String Descriptor 0)

```
44 0 checksum
```

## Return LangIDs

```
45 length_in_bytes [list of 16-bit LangIDs] checksum
```
The Device Pico sends this to the Host Pico to convey the list of LANGIDs the connected control surface device.

## Request String

```
46 3 string_index langid_lsb langid_msb checksum
```
The Device Pico sends this message to the Host Pico to request a particular string in a particular language.

## Return String

```
47 length string_index langid_lsb langid_msb num_bytes utf16_le_string checksum
```
The Host Pico sends this message to the Device Pico in response to a Request String message.

## Return Navigation Buttons Status Byte
```
50 Nav_button_status_byte checksum
```
The host sends to the device the status of the navigation buttons; bit set is pressed.

| Bit | Description |
| --- | ----------- |
|  0  | UP          |
|  1  | DOWN        |
|  2  | LEFT        |
|  3  | RIGHT       |
|  4  | SELECT      |
|  5  | BACK        |
|  6  | SHIFT       |

# References
The [This site](https://sites.google.com/view/mackiecontroluniversaldiyguide/home) 
documents the way MC uses MIDI. The Logic Control guide documents most of what you need. MC protocol uses 0x14 instead of 0x10 and 0x15 instead of 0x11 for the product IDs. There are some messages that are not documented here. I found some of these out by web search engine in forums, a look at the [Cakewalk Control Surface SDK](https://github.com/Cakewalk/Cakewalk-Control-Surface-SDK), and when all else failed, by using my DAW and [MidiView](https://hautetechnique.com/midi/midiview/).

# I/O Pin Usage
## USB Device Pico
The USB is configured in USB Device mode. This Pico controls 9 SSD1306-base 128x64 OLED modules over I2C. Some OLED modules have solder jumpers that allow selection of address 0x3C or 0x3D, but not all do. There is one OLED module per channel strip. The graphics driver code for this project allows you to update all 9 displays in parallel using 8 I2C ports from 2 PIO modules plus I2C1. The USB Device Pico communicates with the USB Host Pico via UART1. UART0 is used for debug console.

- UART0 on pins GP0 and GP1 is used with the picoprobe for debug console
- I2C1 on pins GP2 and GP3 is wired to the timecode OLED
- UART1 on pins on GP4 and GP5 is wired to the USB Host Pico
- PIO0 on pins GP6 and GP7 is configured for I2C and is wired to channel CH 1 OLED
- PIO0 on pins GP8 and GP9 is configured for I2C and is wired to channel CH 2 OLED
- PIO0 on pins GP10 and GP11 is configured for I2C and is wired to channel CH 3 OLED
- PIO0 on pins GP12 and GP13 is configured for I2C and is wired to channel CH 4 OLED
- PIO0 on pins GP14 and GP15 is configured for I2C and is wired to channel CH 5 OLED
- PIO0 on pins GP16 and GP17 is configured for I2C and is wired to channel CH 6 OLED
- PIO0 on pins GP18 and GP19 is configured for I2C and is wired to channel CH 7 OLED
- PIO0 on pins GP20 and GP21 is configured for I2C and is wired to channel CH 8 OLED
- 4 Unused pins GP22, GP26-GP28

## USB Host Pico
The USB is configured in USB Host mode. It connects to the control surface's USB port. It also controls 8 VPot press switch (optional if
your controller has these) and one button that DAWs usually support for choosing between
Bars/Beats/Subdivisions/ticks or SMPTE timecode (SMPTE/Beats button). The USB Device Pico communicates with the USB Host Pico via UART1. UART0 is used for debug console.

- UART0 on pins GP0 and GP1 is used with the picoprobe for debug console
- I2C1 on pins GP2 and GP3 is wired to the time display OLED
- UART1 on pins on GP4 and GP5 is wired to the USB Host Pico
- pin GP7 is the SMPTE/Beats button
- pins GP8-GP15 are for VPot press switches for channels 1-8
- 10 unused pins GP16-GP22, GP26-28

# Settings UI

## Entering the Settings UI
The first time the Host Pico connects to a device with a previously unknown VID and PID,
the Device Pico will start the Settings UI.

The other way to enter the Settings UI is to press the Settings button (GP22? TBD)

## Settings Menu
The Settings Menu is drawn in 12 x 6 font on the Timecode OLED. There is room for 21 characters on a single row. There are at most 5 rows of text, so the following menu
can be displayed without scrolling

```
123456789012345678901
Exit without saving
Save and exit
MC MIDI Port:      NN
Fader Soft Pickup: On
Remap MC note: XX: XX
```

## Operation
There are two user controls: the Settings Button and any rotary encoder on the connected device. The UI always enters with "Exit without saving" displayed in reverse text. If you
rotate the encoder clockwise, the first line will display in normal text and the next
line will display in reverse text. For each increment command from the encoder, the menu
will display the next line highlighted. When the menu highlights the last line, further
increments will do nothing. To highlight a line in the other direction, rotate the
encoder counter-clockwise.

The Settings button becomes a select button once the menu is displayed. If you press the
Settings button, it will execute the function on the highlighted line. The first two lines
are self explanatory. For setting MC MIDI Port numbers, pressing the select button will
cause the text up to the colon to display in normal text but the port number field will
be in reverse text. Rotate the encoder to choose the desired value and press the Settings
button when done (the whole line will be displayed in reverse text again). Similar for
remap note number but the first button press selects the note number and the second button
press selects the remap note number after the colon.

# Future Features
## Message Filter/Converter
Many of the MC implementations are not quite right or not quite what you want. The filter/converter block will convert messages from the controller into proper MC protocol and convert MC protocol into messages the controller better understands. This block could also convert control surfaces that are not compatible with MC protocol to MC protocol. Note control
surfaces can include MIDI keyboards that send slider messages if you want to use a keyboard's
sliders to control your mix.

## Add Rotary Encoders
Many control surfaces don't have rotary encoders for VPots or Jog Wheels. Some controllers implement VPots using potentiometers, which is not as nice as using a real rotary encoder. There are just enough pins on the device side Pico to add 8 encoders for VPots. Two of the 10
available pins on the Host Pico could be used for a jog wheel encoder. The VPot press
switches could be replaced with the press switch function of the encoders; you can use
the same 8 pins on the Host Pico for this.

## Add Transport Buttons
Some control surfaces do not have transport control (play, stop, record, etc.) buttons. You
can use some of the unused GPIO pins on the USB Host Pico for adding more buttons.



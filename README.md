PSGroove
========

This is the PSGroove, an open-source reimplementation of the psjailbreak exploit for
non-usb ATMega chips. (Like those used in arduinos.)

It is known to work on:

- ATMega1280
- ATMega168
- ATMega328p

... and maybe more.

**This software is not intended to enable piracy, and such features
have been disabled.  This software is intended to allow the execution
of unsigned third-party apps and games on the PS3.**

No one involved in maintaining the psgroove git is responsible for or has any involvement with any existing usb dongles sporting "psgroove" in its name. Thank you for your understanding.

Cloning
-------
    git clone git://github.com/psgroove/psgroove.git

If you don't have PPU-GCC installed, make might get confused and refuse to build. To fix this do something like:
    cd PL3
    make clean
    git checkout .
    cd ..

Make should now work as expected and use the precompiled PL3 payloads.

Configuring
-----------
This version of PSGroove has been modified to directly use PL3 payloads instead of a single hardcoded Payload for much greater flexablity.

Edit Makefile to reflect your firmware version (3_41, 3_01, 3_10 and 3_15 are currently supported) and board.

Alternately, you can just use the build_hex.sh to automatically build hex files for all supported boards and firmware versions.

By default PSGroove is configured to use the dev PL3 payload which matches the peek/poke payload that PSGroove used to have. You can select another PL3 payload by changing the PAYLOAD define in descriptor.h

Chip and board selection can usually be handled in the Makefile.
In particular, update the MCU, BOARD, and F_CPU lines.  Suggested values:

Arduino Mega

    MCU = atmega1280
    BOARD = ArduinoMega
    F_CLOCK = 16000000

Arduino Duemilanove w/ atmega168

    MCU = atmega168
    BOARD = ArduinoDuemilanove
    F_CLOCK = 16000000

Arduino Duemilanove w/ atmega328p

    MCU = atmega328p
    BOARD = ArduinoDuemilanove
    F_CLOCK = 16000000

Board-specific notes
--------------------
See README in subfolder for Boards corresponding to your particular board.

Building
--------
On Linux, use the AVR GCC toolchain (Debian/Ubuntu package: gcc-avr).
On Windows, WinAVR should do the trick.

    make clean
    make


Programming
-----------

To program, just edit the programming options section of the Makefile
to match your particular board programming setup. You will need avrdude
on your path. Then do:

    make program
    
For arduino users, you can get some hints as to how to configure the
programming options by checking what the arduino IDE does.

1. Edit your arduino preferences file setting upload.verbose to true.
2. Open the arduino ide
3. Create an empty sketch
4. Upload to your board, and check the output.

From the IDE's output you should be able to determine what port to use
and other useful information about configuring the options.

Using
-----
To use this exploit:
  
* Hard power cycle your PS3 (using the switch in back, or unplug it)
* Plug the dongle into your PS3.
* Press the PS3 power button, followed quickly by the eject button.

After a few seconds, the first LED on your dongle should light up.
After about 5 seconds, the second LED will light up (or the LED will
just go off, if you only have one).  This means the exploit worked!
You can see the new "Install Package Files" menu option in the game
menu.


Notes
-----
A programmed dongle won't enumerate properly on a PC, so don't worry
about that.


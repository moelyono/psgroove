#!/bin/bash

#####################################################
# Change atmega1280 to your chip (e.g. atmega328p). #
DEVICE=atmega1280
#####################################################

#####################################################
# Mac OSX Only. Set this to the location of your    #
# Arduino.app.                                      #
MAC_ARDUINO_LOCATION=/Applications
#####################################################

AVRDUDE=avrdude
PROGRAMMER=stk500v1
EXTRA_OPTS=-b57600
PORT=/dev/ttyUSB0
HEX_FILE=psgroove.hex
PLATFORM=`uname`

if [[ "$PLATFORM" == 'Darwin' ]]; then
   MAC_AVR_ROOT=$MAC_ARDUINO_LOCATION/Arduino.app/Contents/Resources/Java/hardware/tools/avr
   if [[ -e $MAC_AVR_ROOT ]]; then
      AVRDUDE=$MAC_AVR_ROOT/bin/avrdude
      AVRDUDE="$AVRDUDE -C$MAC_AVR_ROOT/etc/avrdude.conf"
   fi

   if [[ `ls /dev/tty.usbserial-*` ]]; then
      PORT=`ls /dev/tty.usbserial-* | head -1`
   else
      echo "Can't find your arduino!"
      exit -1
   fi
elif [[ "$PLATFORM" == 'Linux' ]]; then
   if [[ `ls /dev/ttyUSB*` ]]; then
      PORT=`ls /dev/ttyUSB* | head -1`
   else
      echo "Can't find your arduino!"
      exit -1
   fi
else
   echo "Unknown platform $PLATFORM."
   exit -1
fi

echo $AVRDUDE -p$DEVICE -P$PORT -c$PROGRAMMER $EXTRA_OPTS -U flash:w:$HEX_FILE


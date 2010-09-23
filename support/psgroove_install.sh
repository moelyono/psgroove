#!/bin/bash

AVRDUDE=avrdude
DEVICE=atmega1280
PROGRAMMER=stk500v1
EXTRA_OPTS=-b57600
PORT=/dev/ttyUSB0
HEX_FILE=psgroove.hex
PLATFORM=`uname`

if [[ "$PLATFORM" == 'Darwin' ]]; then
   if [[ -e /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avrdude ]]; then
      AVRDUDE=/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avrdude
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

$AVRDUDE -p$DEVICE -P$PORT -c$PROGRAMMER $EXTRA_OPTS -U flash:w:$HEX_FILE


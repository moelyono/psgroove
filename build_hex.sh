#!/bin/bash
#
# Copyright (C) Youness Alaoui (KaKaRoTo)
#
# This software is distributed under the terms of the GNU General Public
# License ("GPL") version 3, as published by the Free Software Foundation.
#

all_targets="mega duemilanove168 \
             duemilanove168p duemilanove328p"

i=1
for target in ${all_targets}; do
  let ${target}=$i
  let i++
done

mcu[$mega]=atmega1280
board[$mega]=ArduinoMega
mhz_clock[$mega]=16
name[$mega]="Arduino Mega"

mcu[$duemilanove168]=atmega168
board[$duemilanove168]=ArduinoDuemilanove
mhz_clock[$duemilanove168]=16
name[$duemilanove168]="Arduino Duemilanove atmega168"

mcu[$duemilanove168p]=atmega168p
board[$duemilanove168p]=ArduinoDuemilanove
mhz_clock[$duemilanove168p]=16
name[$duemilanove168p]="Arduino Duemilanove atmega168p"

mcu[$duemilanove328p]=atmega328p
board[$duemilanove328p]=ArduinoDuemilanove
mhz_clock[$duemilanove328p]=16
name[$duemilanove328p]="Arduino Duemilanove atmega328p"

while [ "x$1" != "x" ]; do
  targets="$targets ${1}"
  shift
done
if [ "x$targets" == "x" ]; then
  for i in ${all_targets}; do
    targets="$targets ${i}"
  done
fi

echo "Building for targets : $targets"

rm -rf psgroove_hex/
mkdir psgroove_hex
make clean_list > /dev/null

for target in ${targets}; do
  for firmware in 3.01 3.10 3.15 3.41 ; do
    firmware=${firmware/./_}
    low_board=`echo ${board[${!target}]} | awk '{print tolower($0)}'`
    filename="psgroove_${low_board}_${mcu[${!target}]}_${mhz_clock[${!target}]}mhz_firmware_${firmware}"
    echo "Compiling $filename for ${name[${!target}]}"
    make TARGET=$filename MCU=${mcu[${!target}]} BOARD=${board[${!target}]} F_CPU=${mhz_clock[${!target}]}000000 FIRMWARE_VERSION=${firmware} > /dev/null || exit 1
    mkdir -p "psgroove_hex/${name[${!target}]}"
    mv *.hex "psgroove_hex/${name[${!target}]}/"
    make clean_list TARGET=$filename > /dev/null
  done
done


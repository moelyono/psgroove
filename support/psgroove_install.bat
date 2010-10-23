rem ###########################################################
rem Change atmega1280 to your particular chip (e.g. atmega328p)
set DEVICE=atmega1280
rem ###########################################################
rem Change this to change your board type.
set BOARD=arduinomega
rem ###########################################################
rem The firmware version of your PS3 (e.g 3_01)
set FIRMWARE=3_41
rem ##########################################################
rem Change the COM3 to your COM port. Check the arduino IDE in
rem Tools->Serial Port for the correct port.
set PORT=COM3
rem ###########################################################

set PROGRAMMER=stk500v1
set EXTRA_OPTS=-b57600 -D
set HEX_FILE=psgroove_%BOARD%_%DEVICE%_16mhz_firmware_%FIRMWARE%.hex

hardware\tools\avr\bin\avrdude -Chardware\tools\avr\etc\avrdude.conf -p%DEVICE% -c%PROGRAMMER% -P\\.\%PORT% %EXTRA_OPTS% -Uflash:w:%HEX_FILE%:i

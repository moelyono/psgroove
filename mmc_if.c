
/* ***********************************************************************
**
**  Copyright (C) 2006  Jesper Hansen <jesper@redegg.net>
**
**
**  Interface functions for MMC/SD cards
**
**  File mmc_if.h
**
**  Hacked by Michael Spiceland at http://tinkerish.com to support
**  writing as well.
**  Hacked again by Tim Wu @ http://github.com/timwu to add buffered
**  read support and sub-sector sized reads.
**
*************************************************************************
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software Foundation,
**  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
*************************************************************************/

/** \file mmc_if.c
	Simple MMC/SD-card functionality
*/

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include "mmc_if.h"
#include "oddebug.h"

#define MAX(x,y) (x) > (y) ? (x) : (y)
#define MIN(x,y) (x) < (y) ? (x) : (y)

struct {
   uint8_t buf[READ_BUF_SZ];
   uint16_t block;
   uint32_t lba;
} read_buffer;

/** Hardware SPI I/O.
	\param byte Data to send over SPI bus
	\return Received data from SPI bus
*/
uint8_t spi_byte(uint8_t byte)
{
	SPDR = byte;
	while(!(SPSR & (1<<SPIF)))
	{}
	return SPDR;
}

/** Send a command to the MMC/SD card.
	\param command	Command to send
	\param px	Command parameter 1
	\param py	Command parameter 2
*/
uint8_t mmc_send_command(uint8_t command, uint32_t arg)
{
   ENABLE_CS();

	spi_byte(0xff);			// dummy byte

	spi_byte(command | 0x40);

   spi_byte(arg >> 24);
   spi_byte(arg >> 16);
   spi_byte(arg >> 8);
   spi_byte(arg & 0xff);

	spi_byte(0x95);			// correct CRC for first command in SPI
							// after that CRC is ignored, so no problem with
							// always sending 0x95
                     //
   uint8_t R1;
   while ((R1 = spi_byte(0xff)) & 0x80);
   return R1;
}


/** Get Token.
	Wait for and return a non-ff token from the MMC/SD card
	\return The received token or 0xFF if timeout
*/
uint8_t mmc_get(void)
{
	uint16_t i = 0xffff;
	uint8_t b = 0xff;

	while ((b == 0xff) && (--i))
	{
		b = spi_byte(0xff);
	}
	return b;

}

/** Get Datatoken.
	Wait for and return a data token from the MMC/SD card
	\return The received token or 0xFF if timeout
*/
uint8_t mmc_datatoken(void)
{
	uint16_t i = 0xffff;
	uint8_t b = 0xff;

	while ((b != 0xfe) && (--i))
	{
		b = spi_byte(0xff);
	}
	return b;
}


/** Finish Clocking and Release card.
	Send 10 clocks to the MMC/SD card
 	and release the CS line
*/
void mmc_clock_and_release(void)
{
	uint8_t i;

	// SD cards require at least 8 final clocks
	for(i=0;i<10;i++)
		spi_byte(0xff);

   DISABLE_CS();
}

/** Read MMC/SD sector.
 	Read a single 512 byte sector from the MMC/SD card
	\param lba	Logical sectornumber to read
	\param buffer	Pointer to buffer for received data
	\return 0 on success, -1 on error
*/
int mmc_readsector(uint32_t lba, uint8_t *buffer)
{
	uint16_t i;

	// send read command and logical sector address
	mmc_send_command(17, lba * SECTOR_SZ);

	if (mmc_datatoken() != 0xfe)	// if no valid token
	{
		mmc_clock_and_release();	// cleanup and
		//for (i=0;i<512;i++)				// we don't want a false impression that everything was fine.
    		//	*buffer++ = 0x00;
   		return -1;					// return error code
	}

	for (i=0;i<512;i++)				// read sector data
    	*buffer++ = spi_byte(0xff);

	spi_byte(0xff);					// ignore dummy checksum
	spi_byte(0xff);					// ignore dummy checksum

	mmc_clock_and_release();		// cleanup

	return 0;						// return success
}

static int _mmc_read(uint8_t *buffer, uint32_t lba, uint16_t offset, uint16_t length)
{
   if (offset + length > SECTOR_SZ)
      return -1; // Reading more than a sector is not allowed.

   uint8_t result;
   uint8_t retries;

	// send read command and logical sector address
#ifdef USE_SET_BLOCKLEN
   if ((result = mmc_send_command(17, lba * SECTOR_SZ + offset))) {
#else
   uint16_t trailingBytes = SECTOR_SZ - offset - length;
	if ((result = mmc_send_command(17,lba * SECTOR_SZ))) {
#endif
      DBGX1("Read failed: ", &result, 1);
      return -1;
   }

   retries = 0;
   while(spi_byte(0xff) != 0xfe) {
      if(retries++ == 0xff) {
         DBGMSG1("Timed out waiting for start block datatoken.");
         mmc_clock_and_release();
         return -1;
      }
   }

#ifdef USE_SET_BLOCKLEN
   while(length--) *buffer++ = spi_byte(0xff);
#else
   while(offset--) spi_byte(0xff); // Fast-forward to the desired offset
   while(length--) *buffer++ = spi_byte(0xff); // Read length bytes
   while(trailingBytes--) spi_byte(0xff); // Ignore the rest of the bytes in the sector.
#endif

	spi_byte(0xff);					// ignore dummy checksum
	spi_byte(0xff);					// ignore dummy checksum

	mmc_clock_and_release();		// cleanup

   return 0;
}

int mmc_read(uint8_t *buffer, uint32_t lba, uint16_t offset, uint16_t length)
{
   while (length) {
      uint16_t bufBlock = offset / READ_BUF_SZ;
      if (lba == read_buffer.lba &&
          bufBlock == read_buffer.block) {
         // Cache hit
         uint16_t bufOffset = offset - (read_buffer.block * READ_BUF_SZ);
         uint16_t bufReadLen = MIN(READ_BUF_SZ - bufOffset, length);
         memcpy(buffer, read_buffer.buf + bufOffset, bufReadLen);
         length -= bufReadLen;
         offset += bufReadLen;
         buffer += bufReadLen;
      } else {
         if (_mmc_read(read_buffer.buf, lba,
                       bufBlock * READ_BUF_SZ, READ_BUF_SZ)) {
            DBGMSG1("Failed to read in block.");
            return -1;
         }
         read_buffer.lba = lba;
         read_buffer.block = bufBlock;
      }
   }
   return 0;
}

/************************** MMC get response **************************************/
/**** Repeatedly reads the MMC until we get the response we want or timeout ****/
/* this function taken from the PIC CCS example */
int mmc_response(unsigned char response)
{
        unsigned long count = 0xFFFF;           // 16bit repeat, it may be possible to shrink this to 8 bit but there is not much point

        while(spi_byte(0xFF) != response && --count > 0);

        if(count==0) return 1;                  // loop was exited due to timeout
        else return 0;                          // loop was exited before timeout
}

/** Write MMC/SD sector.
 	Write a single 512 byte sector from the MMC/SD card
	\param lba	Logical sectornumber to write
	\param buffer	Pointer to buffer to write from
	\return 0 on success, -1 on error
*/
int mmc_writesector(uint32_t lba, uint8_t *buffer)
{
	uint16_t i;

	// send read command and logical sector address
	mmc_send_command(24, lba * SECTOR_SZ);

	// need wait for resonse here
	if((mmc_response(0x00))==1)
	{
		mmc_clock_and_release();	// cleanup and
		return -1;
	}

	// need to send token here
	spi_byte(0xfe);					// send data token /* based on PIC code example */

	for (i=0;i<512;i++)				// write sector data
    		spi_byte(*buffer++);

	spi_byte(0xff);					// ignore dummy checksum
	spi_byte(0xff);					// ignore dummy checksum

	// do we check the status here?
	if((spi_byte(0xFF)&0x0F)!=0x05) return -1;

	i = 0xffff;						// max timeout
	while(!spi_byte(0xFF) && (--i)){;} // wait until we are not busy

	mmc_clock_and_release();		// cleanup

	return 0;						// return success
}


/** Init MMC/SD card.
	Initialize I/O ports for the MMC/SD interface and
	send init commands to the MMC/SD card
	\return 0 on success, other values on error
*/
uint8_t mmc_init(void)
{
	int i;

   read_buffer.lba = 0xffffffff; //Assume we'll never get to the last LBA block

	// setup I/O ports

	SPI_PORT &= ~((1 << MMC_SCK) | (1 << MMC_MOSI));	// low bits
	SPI_PORT |= (1 << MMC_MISO);						// high bits
	SPI_DDR  |= (1<<MMC_SCK) | (1<<MMC_MOSI);			// direction


	MMC_CS_PORT |= (1 << MMC_CS);	// Initial level is high
	MMC_CS_DIR  |= (1 << MMC_CS);	// Direction is output


	// also need to set SS as output
#if defined(__AVR_ATmega8__)
	// is already set as CS, but we set it again to accomodate for other boards
	SPI_DDR |= (1<<2);
#else
	SPI_DDR |= (1<<0);			// assume it's bit0 (mega128, portB and others)
#endif

	SPCR = (1<<MSTR)|(1<<SPE); // enable SPI
   
   SPCR |= (1<<SPR0)|(1<<SPR1);	// Set low speed mode
	SPSR = 0;					// unset double speed

	for(i=0;i<10;i++)			// send 80 clocks while card power stabilizes
		spi_byte(0xff);

   uint8_t retries = 0;
   while (mmc_send_command(0,0) != 1) {
      if (retries++ == 0xff) {
         DBGMSG1("Timed out resetting SD card.");
         return -1;
      }
   }
   retries = 0;
   while (mmc_send_command(1,0) != 0) {
      if (retries++ == 0xff) {
         DBGMSG1("Timed out initializing SD card.");
         return -1;
      }
   }

   // Increase SPI speed.
   SPCR &= ~((1<<SPR0) | (1<<SPR1));
   SPSR |= (1<<SPI2X);

#ifdef USE_SET_BLOCKLEN
   if (mmc_send_command(16, READ_BUF_SZ)) {
      DBGMSG1("Failed to set block length.");
      return -1;
   }
#endif

   mmc_clock_and_release();		// clean up

   DBGMSG1("Finished initializing mmc.");
	return 0;
}



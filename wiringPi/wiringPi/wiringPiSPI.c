/*
 * wiringPiSPI.c:
 *	Simplified SPI access routines
 *	Copyright (c) 2012 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringPi.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */


#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "wiringPiSPI.h"


// The SPI bus parameters
//	Variables as they need to be passed as pointers later on

static char       *spiDev0 = "/dev/spidev0.0" ;
static char       *spiDev1 = "/dev/spidev0.1" ;
static uint8_t     spiMode   = SPI_MODE_0;
static uint8_t     spiBPW    = 8 ;
static uint16_t    spiDelay  = 0;

static uint32_t    spiSpeeds [2] ;
static int         spiFds [2] ;


/*
 * wiringPiSPIGetFd:
 *	Return the file-descriptor for the given channel
 *********************************************************************************
 */

int wiringPiSPIGetFd (int channel)
{
  return spiFds [channel & 1] ;
}


/*
 * wiringPiSPIDataRW:
 *	Write and Read a block of data over the SPI bus.
 *	Note the data is being read into the transmit buffer, so will
 *	overwrite it!
 *	This is also a full-duplex operation.
 *********************************************************************************
 */

int wiringPiSPIDataRW (int channel, unsigned char *data, int length)
{
  struct spi_ioc_transfer spi[length] ;
  int i = 0;
  
  channel &= 1 ;

  for (i = 0; i < length; i++){
	  memset(&spi[i], 0, sizeof (spi[i]));
	  spi[i].tx_buf        = (unsigned long)(data + i) ;
	  spi[i].rx_buf        = (unsigned long)(data + i) ;
	  spi[i].len			= sizeof(*(data + i)) ;
	  //spi.len           = len ;
	  spi[i].delay_usecs   = spiDelay ;
	  spi[i].speed_hz      = spiSpeeds [channel] ;
	  spi[i].bits_per_word = spiBPW ;
  }

  return ioctl (spiFds [channel], SPI_IOC_MESSAGE(length), &spi) ;
}


/*
 * wiringPiSPISetup:
 *	Open the SPI device, and set it up, etc.
 *********************************************************************************
 */

int wiringPiSPISetup (int channel, int speed, int mode)
{
  int fd ;

  channel &= 1 ;

  if ((fd = open (channel == 0 ? spiDev0 : spiDev1, O_RDWR)) < 0)
    return -1 ;

  spiSpeeds [channel] = speed ;
  spiFds    [channel] = fd ;
  
  switch(mode){
  case 0:
	spiMode = SPI_MODE_0;
	break;
  case 1:
  	spiMode = SPI_MODE_1;
	break;
  case 2:
  	spiMode = SPI_MODE_2;
	break;
  case 3:
  	spiMode = SPI_MODE_3;
	break;
}

// Set SPI parameters.
//	Why are we reading it afterwriting it? I've no idea, but for now I'm blindly
//	copying example code I've seen online...

  if (ioctl (fd, SPI_IOC_WR_MODE, &spiMode)         < 0) return -1 ;
  if (ioctl (fd, SPI_IOC_RD_MODE, &spiMode)         < 0) return -1 ;

  if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0) return -1 ;
  if (ioctl (fd, SPI_IOC_RD_BITS_PER_WORD, &spiBPW) < 0) return -1 ;

  if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)   < 0) return -1 ;
  if (ioctl (fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)   < 0) return -1 ;

  return fd ;
}

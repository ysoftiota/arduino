/*
  SPIFirmata.h
  Copyright (C) 2017 Jeff Hoefs. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated January 7th, 2017
*/

#ifndef SPIFirmata_h
#define SPIFirmata_h

#include <Firmata.h>
#include "FirmataFeature.h"
#include <SPI.h>

#define FIRMATA_SPI_FEATURE

// following 2 defines belong in FirmataConstants.h, but declaring them here
// since this implementation is still a prototype
#define SPI_DATA                    0x68
#define PIN_MODE_SPI                0x0C

// SPI mode bytes
#define SPI_CONFIG                  0x00
#define SPI_BEGIN_TRANSACTION       0x01
#define SPI_END_TRANSACTION         0x02
#define SPI_TRANSFER                0x03
#define SPI_REPLY                   0x04
#define SPI_END                     0x05

// transfer options
#define SPI_TRANSFER_OPTS_MASK      0x03
#define SPI_READ_WRITE              0x00
#define SPI_READ_ONLY               0x01
#define SPI_WRITE_ONLY              0x02

// pinOptions
#define SPI_CS_DISABLE_MASK         0x04
#define SPI_CS_START_ONLY_MASK      0x08
#define SPI_CS_END_ONLY_MASK        0x10
#define SPI_CS_ACTIVE_EDGE_MASK     0x20
#define SPI_CS_TOGGLE_MASK          0x40

#define SPI_CHANNEL_MASK            0x03
#define SPI_DEVICE_ID_MASK          0x7C
#define SPI_BIT_ORDER_MASK          0x01

namespace {
// TODO - check Teensy and other non SAM, SAMD or AVR variants
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  inline BitOrder getBitOrder(uint8_t value) {
    if (value == 0) return LSBFIRST;
    return MSBFIRST; // default
  }
#else
  inline uint8_t getBitOrder(uint8_t value) {
    if (value == 0) return LSBFIRST;
    return MSBFIRST; // default
  }
#endif

  inline uint8_t getDataMode(uint8_t value) {
    if (value == 1) {
      return SPI_MODE1;
    } else if (value == 2) {
      return SPI_MODE2;
    } else if (value == 3) {
      return SPI_MODE3;
    }
    return SPI_MODE0; // default
  }

}

class SPIFirmata: public FirmataFeature
{
  public:
    SPIFirmata();
    boolean handlePinMode(byte pin, int mode);
    void handleCapability(byte pin);
    boolean handleSysex(byte command, byte argc, byte *argv);
    void reset();

  private:
    signed char mCsPin;
    byte mDeviceId;

    void readWrite(byte channel, byte numBytes, byte argc, byte *argv);
    void readOnly(byte channel, byte numBytes);
    void writeOnly(byte channel, byte numBytes, byte argc, byte *argv);
    void reply(byte channel, byte numBytes, byte *buffer);
};

#endif /* SPIFirmata_h */

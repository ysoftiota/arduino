/*
  SPIFirmata.h
  Copyright (C) 2017 Jeff Hoefs. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated January 8th, 2017
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
#define SPI_TRANSFER_MODES_MASK     0x03
#define SPI_READ_WRITE              0x00
#define SPI_READ_ONLY               0x01
#define SPI_WRITE_ONLY              0x02

// pinOptions
#define SPI_CS_DISABLE_MASK         0x04
#define SPI_CS_START_ONLY_MASK      0x08
#define SPI_CS_END_ONLY_MASK        0x10
#define SPI_CS_TOGGLE_MASK          0x20

#define SPI_CHANNEL_MASK            0x03
#define SPI_DEVICE_ID_MASK          0x7C
#define SPI_BIT_ORDER_MASK          0x01

#define SPI_CS_ACTIVE_LOW           0x00
#define SPI_CS_ACTIVE_HIGH          0x01

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
    bool handlePinMode(uint8_t pin, int mode);
    void handleCapability(uint8_t pin);
    bool handleSysex(uint8_t command, uint8_t argc, uint8_t *argv);
    void reset();

  private:
    // TODO - each of these properties should be in an array of channels
    int8_t mCsPin;
    bool mCsActiveState;
    uint8_t mDeviceId;

    void init();

    void readWrite(uint8_t channel, uint8_t numBytes, uint8_t argc, uint8_t *argv);
    void readOnly(uint8_t channel, uint8_t numBytes);
    void writeOnly(uint8_t channel, uint8_t numBytes, uint8_t argc, uint8_t *argv);
    void reply(uint8_t channel, uint8_t numBytes, uint8_t *buffer);
};

#endif /* SPIFirmata_h */

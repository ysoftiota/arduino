/*
  SPIFirmata.cpp
  Copyright (C) 2017 Jeff Hoefs. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated January 8th, 2017
*/

#include "SPIFirmata.h"

SPIFirmata::SPIFirmata()
{
  init();
}

void SPIFirmata::init()
{
  mDeviceId = 0;
  mCsPin = -1;
  mCsActiveState = SPI_CS_ACTIVE_LOW; // default
}

bool SPIFirmata::handlePinMode(uint8_t pin, int mode)
{
  // There is no reason for a user to manually set the SPI pin modes
  return false;
}

void SPIFirmata::handleCapability(uint8_t pin)
{
  // ignore SS pin for now
  if (IS_PIN_SPI(pin) && pin != SS) {
    Firmata.write(PIN_MODE_SPI);
    // would actually use a value that corresponds to a specific pin (MOSI, MISO, SCK)
    // for now, just set to 1
    Firmata.write(1);
  }
}

bool SPIFirmata::handleSysex(uint8_t command, uint8_t argc, uint8_t *argv)
{
  if (command == SPI_DATA) {
    uint8_t mode = argv[0];
    // not using channel yet
    uint8_t channel = argv[1] & SPI_CHANNEL_MASK;

    switch (mode) {
      case SPI_CONFIG:
        SPI.begin();
        // SPI pin states are configured by SPI.begin, but we still register them with Firmata.
        Firmata.setPinMode(MOSI, PIN_MODE_SPI);
        Firmata.setPinMode(MISO, PIN_MODE_SPI);
        Firmata.setPinMode(SCK, PIN_MODE_SPI);
        // ignore SS for now
        //Firmata.setPinMode(SS, PIN_MODE_SPI);
        break;
      case SPI_BEGIN_TRANSACTION:
      {
        mDeviceId = argv[1] >> 2;
        uint8_t bitOrder = argv[2] & SPI_BIT_ORDER_MASK;
        uint8_t dataMode = argv[2] >> 1;
        uint32_t clockSpeed = (uint32_t)argv[3] | ((uint32_t)argv[4] << 7) |
            ((uint32_t)argv[5] << 14) | ((uint32_t)argv[6] << 21) | ((uint32_t)argv[7] << 28);

        if (argc > 8) {
          mCsPin = argv[8];
          pinMode(mCsPin, OUTPUT);

          if (argv[9] != END_SYSEX) {
            mCsActiveState = argv[9];
          } else {
            // set default
            mCsActiveState = SPI_CS_ACTIVE_LOW;
          }
          // set CS pin to opposite of active state
          digitalWrite(mCsPin, !mCsActiveState);

          // protect the CS pin
          // TODO - decide if this is the best approach. If PIN_MODE_SPI is set, the user cannot
          // manually control the CS pin using DIGITAL_MESSAGE.
          Firmata.setPinMode(mCsPin, PIN_MODE_SPI);
        }

        SPISettings settings(clockSpeed, getBitOrder(bitOrder), getDataMode(dataMode));
        SPI.beginTransaction(settings);
        break;
      }
      case SPI_END_TRANSACTION:
        SPI.endTransaction();
        break;
      case SPI_TRANSFER:
      {
        uint8_t transferOptions = argv[2] & SPI_TRANSFER_MODES_MASK;
        uint8_t numBytes = argv[3];

        bool csIsActive = true;
        bool csStartOnly = false;
        bool csEndOnly = false;

        //bool csToggle = false;

        if (mCsPin >= 0) {
          if (argv[2] & SPI_CS_DISABLE_MASK) {
            csIsActive = false;
          } else {
            if (argv[2] & SPI_CS_START_ONLY_MASK) csStartOnly = true;
            if (argv[2] & SPI_CS_END_ONLY_MASK) csEndOnly = true;
            // TODO - handle csToggle
            // if (argv[2] & SPI_CS_TOGGLE_MASK) csToggle = true;
          }
        }

        if (mCsPin >= 0 && csIsActive && !csEndOnly) {
          digitalWrite(mCsPin, mCsActiveState);
        }

        if (transferOptions == SPI_READ_WRITE) {
          readWrite(channel, numBytes, argc, argv);
        } else if (transferOptions == SPI_READ_ONLY) {
          readOnly(channel, numBytes);
        } else if (transferOptions == SPI_WRITE_ONLY) {
          writeOnly(channel, numBytes, argc, argv);
        } else {
          // TODO - handle error
          Firmata.sendString("No transferOptions set");
        }

        // TODO - ideally this should be called before the SPI_REPLY message is sent.
        if (mCsPin >= 0 && csIsActive && !csStartOnly) {
          digitalWrite(mCsPin, !mCsActiveState);
        }
        break; // SPI_TRANSFER
      }
      case SPI_END:
        SPI.end();
        break;
    } // end switch
    return true;
  }
  return false;
}

void SPIFirmata::reset()
{
  init();
}

void SPIFirmata::readWrite(uint8_t channel, uint8_t numBytes, uint8_t argc, uint8_t *argv)
{
  uint8_t offset = 4; // mode + channel + opts + numBytes
  uint8_t buffer[numBytes];
  uint8_t bufferIndex = 0;
  if (numBytes * 2 != argc - offset) {
    // TODO - handle error
    Firmata.sendString("fails numBytes test");
  }
  for (uint8_t i = 0; i < numBytes * 2; i += 2) {
    bufferIndex = (i + 1) / 2;
    buffer[bufferIndex] = argv[i + offset + 1] << 7 | argv[i + offset];
  }
  SPI.transfer(buffer, numBytes);

  reply(channel, numBytes, buffer);
}

void SPIFirmata::writeOnly(uint8_t channel, uint8_t numBytes, uint8_t argc, uint8_t *argv)
{
  uint8_t offset = 4;
  uint8_t txValue;
  if (numBytes * 2 != argc - offset) {
    // TODO - handle error
    Firmata.sendString("fails numBytes test");
  }
  for (uint8_t i = 0; i < numBytes * 2; i += 2) {
    txValue = argv[i + offset + 1] << 7 | argv[i + offset];
    SPI.transfer(txValue);
  }
}

void SPIFirmata::readOnly(uint8_t channel, uint8_t numBytes)
{
  uint8_t replyData[numBytes];
  for (uint8_t i = 0; i < numBytes; i++) {
    replyData[i] = SPI.transfer(0x00);
  }
  reply(channel, numBytes, replyData);
}

void SPIFirmata::reply(uint8_t channel, uint8_t numBytes, uint8_t *buffer)
{
  Firmata.write(START_SYSEX);
  Firmata.write(SPI_DATA);
  Firmata.write(SPI_REPLY);
  Firmata.write(mDeviceId << 2 | channel);
  Firmata.write(numBytes);

  for (uint8_t i = 0; i < numBytes; i++) {
    Firmata.write(buffer[i] & 0x7F);
    Firmata.write(buffer[i] >> 7 & 0x7F);
  }

  Firmata.write(END_SYSEX);
}

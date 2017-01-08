/*
  SPIFirmata.cpp
  Copyright (C) 2017 Jeff Hoefs. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated January 7th, 2017
*/

#include "SPIFirmata.h"

SPIFirmata::SPIFirmata()
{
  mDeviceId = 0;
  mCsPin = -1;
}

boolean SPIFirmata::handlePinMode(byte pin, int mode)
{
  // ignore SS pin for now
  if (mode == PIN_MODE_SPI && pin != SS) {
    Firmata.setPinMode(pin, PIN_MODE_SPI);
    return true;
  }
  return false;
}

void SPIFirmata::handleCapability(byte pin)
{
  // ignore SS pin for now
  if (IS_PIN_SPI(pin) && pin != SS) {
    Firmata.write(PIN_MODE_SPI);
    // would actually use a value that corresponds to a specific pin (MOSI, MISO, SCK)
    // for now, just set to 1
    Firmata.write(1);
  }
}

boolean SPIFirmata::handleSysex(byte command, byte argc, byte *argv)
{
  if (command == SPI_DATA) {
    byte mode = argv[0];
    // not using channel yet
    byte channel = argv[1] & SPI_CHANNEL_MASK;

    switch (mode) {
      case SPI_CONFIG:
        SPI.begin();
        break;
      case SPI_BEGIN_TRANSACTION:
      {
        mDeviceId = argv[1] >> 2;
        uint8_t bitOrder = argv[2] & SPI_BIT_ORDER_MASK;
        uint8_t dataMode = argv[2] >> 1;
        uint32_t clockSpeed = (uint32_t)argv[3] | ((uint32_t)argv[4] << 7) | ((uint32_t)argv[5] << 14) |
                          ((uint32_t)argv[6] << 21) | ((uint32_t)argv[7] << 28);

        if (argc > 8) {
          mCsPin = argv[8];
          pinMode(mCsPin, OUTPUT);
          // TODO - need to know if the device is active LOW or active HIGH at this time.
          digitalWrite(mCsPin, HIGH);

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
        byte transferOptions = argv[2] & SPI_TRANSFER_OPTS_MASK;
        byte numBytes = argv[3];

        boolean csIsActive = true;
        byte csStartVal = LOW;
        byte csEndVal = HIGH;
        boolean csStartOnly = false;
        boolean csEndOnly = false;

        //boolean csToggle = false;

        if (mCsPin >= 0) {
          if (argv[2] & SPI_CS_DISABLE_MASK) {
            csIsActive = false;
          } else {
            if (argv[2] & SPI_CS_START_ONLY_MASK) csStartOnly = true;
            if (argv[2] & SPI_CS_END_ONLY_MASK) csEndOnly = true;
            if (argv[2] & SPI_CS_ACTIVE_EDGE_MASK) {
              csStartVal = HIGH;
              csStartVal = LOW;
            }
            // TODO - handle csToggle
            // if (argv[2] & SPI_CS_TOGGLE_MASK) csToggle = true;
          }
        }

        if ((csIsActive || csStartOnly) && !csEndOnly) {
          digitalWrite(mCsPin, csStartVal);
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

        if ((csIsActive || csEndOnly) && !csStartOnly) {
          digitalWrite(mCsPin, csEndVal);
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
  mCsPin = -1;
  mDeviceId = 0;
}

void SPIFirmata::readWrite(byte channel, byte numBytes, byte argc, byte *argv)
{
  byte offset = 4; // mode + channel + opts + numBytes
  byte buffer[numBytes];
  byte bufferIndex = 0;
  if (numBytes * 2 != argc - offset) {
    // TODO - handle error
    Firmata.sendString("fails numBytes test");
  }
  for (byte i = 0; i < numBytes * 2; i += 2) {
    bufferIndex = (i + 1) / 2;
    buffer[bufferIndex] = argv[i + offset + 1] << 7 | argv[i + offset];
  }
  SPI.transfer(buffer, numBytes);

  reply(channel, numBytes, buffer);
}

void SPIFirmata::writeOnly(byte channel, byte numBytes, byte argc, byte *argv)
{
  byte offset = 4;
  byte txValue;
  if (numBytes * 2 != argc - offset) {
    // TODO - handle error
    Firmata.sendString("fails numBytes test");
  }
  for (byte i = 0; i < numBytes * 2; i += 2) {
    txValue = argv[i + offset + 1] << 7 | argv[i + offset];
    SPI.transfer(txValue);
  }
}

void SPIFirmata::readOnly(byte channel, byte numBytes)
{
  byte replyData[numBytes];
  for (byte i = 0; i < numBytes; i++) {
    replyData[i] = SPI.transfer(0x00);
  }
  reply(channel, numBytes, replyData);
}

void SPIFirmata::reply(byte channel, byte numBytes, byte *buffer)
{
  Firmata.write(START_SYSEX);
  Firmata.write(SPI_DATA);
  Firmata.write(SPI_REPLY);
  Firmata.write(mDeviceId << 2 | channel);
  Firmata.write(numBytes);

  for (byte i = 0; i < numBytes; i++) {
    Firmata.write(buffer[i] & 0x7F);
    Firmata.write(buffer[i] >> 7 & 0x7F);
  }

  Firmata.write(END_SYSEX);
}

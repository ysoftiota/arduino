#include <Boards.h>
#include <Firmata.h>

#include <SPI.h>
#include <RH_RF69.h>  // http://www.airspayce.com/mikem/arduino/RadioHead/index.html

#include "RF.h"
#include "UserSysex.h"

// Singleton instance of the radio driver
RH_RF69 rf69(8, 3);

uint8_t rf_buffer[RH_RF69_MAX_MESSAGE_LEN];
uint8_t len = sizeof(rf_buffer);

bool rfInit() {
  if (!rf69.init())
    return false;

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM, No encryption

  if (!rf69.setFrequency(868.0))
    return false;

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  // rf69.setTxPower(14);

  return true;
}

void rfSetKey(uint8_t *key) {
  rf69.setEncryptionKey(key);
}

bool rfSend(uint8_t *data) {
  bool retval = false;
  
  retval = rf69.send(data, sizeof(data));
  retval &= rf69.waitPacketSent();

  return retval;
}

bool rfAvailable() {
  return rf69.available();
}

bool rfRecv() {
  if (!rf69.recv(rf_buffer, &len)) {
    return 0;
  }

  Firmata.sendSysex(USR_CMD_RF, len, (byte *)rf_buffer);

  return len;
}



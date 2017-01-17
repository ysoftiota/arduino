#include <Boards.h>
#include <Firmata.h>

#include "UserSysex.h"
#include "YiPlayRF.h"

byte buffer[MAX_DATA_BYTES];

byte decodeTwoBytes(const byte *data, byte datalen, byte *output) {
  byte i;
  if (datalen > MAX_DATA_BYTES * 2) datalen = MAX_DATA_BYTES * 2;
  for (i = 0; i < datalen/2; i++) {
    output[i] = data[2*i] + (data[2*i + 1] << 7);
  }
  return i;
}

void userSysex(byte command, byte argc, byte *argv) {
  byte bufferLength;
  switch (command) {
    case USR_CMD_ECHO:
      bufferLength = decodeTwoBytes(argv, argc, buffer);
      Firmata.sendSysex(USR_CMD_ECHO, bufferLength, buffer);
      break;
    case USR_CMD_RF_CONFIG:
      if (argc >= 4) {
        uint8_t networkID = argv[0] + (argv[1] << 7);
        uint8_t nodeID = argv[2] + (argv[3] << 7);
        if (argc == 36) {
          for (uint8_t i = 0; i < 16; i++) {
            buffer[i] = argv[4 + 2*i] + (argv[5 + 2*i] << 7);
          }
          setupRadio(networkID, nodeID, (char *)buffer);
        } else setupRadio(networkID, nodeID);
      }
      break;
    case USR_CMD_RF_DATA:
      if (argc >= 2) {
        uint8_t nodeID = argv[0] + (argv[1] << 7);
        bufferLength = decodeTwoBytes(argv + 2, argc - 2, buffer);
        radio.sendWithRetry(nodeID, buffer, bufferLength, RFM69_RETRIES, RFM69_RETRY_DELAY);
      }
      break;
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
    default:
      break;
  }
}


#include <Boards.h>
#include <Firmata.h>

#include "UserSysex.h"
#include "YSoftRF.h"

char buffer[MAX_DATA_BYTES];

void encodeToBuffer(byte argc, byte *argv) {
  byte j = 0;
  byte i = 0;

  while (j < argc && j < (MAX_DATA_BYTES - 1)) {
    buffer[j] = argv[i];
    i++;
    buffer[j] += argv[i];
    i++;
    j++;
  }

  if (buffer[j - 1] != '\0') {
    buffer[j] = '\0';
  }
}

void debugCommand(byte command, byte argc, byte *argv) {
  snprintf(buffer, MAX_DATA_BYTES , "Command %d", command);
  Firmata.sendString(buffer);

  encodeToBuffer(argc, argv);
  Firmata.sendString(buffer);
}

void userSysex(byte command, byte argc, byte *argv) {
  debugCommand(command, argc, argv);

  switch (command) {
    case USR_CMD_ECHO:
      encodeToBuffer(argc, argv);
      Firmata.sendSysex(USR_CMD_ECHO, strlen(buffer), (byte *)buffer);
      break;
    case USR_CMD_RF_CONFIG:
      if (argc >= 4) {
        uint8_t networkID = argv[0] + (argv[1] << 7);
        uint8_t nodeID = argv[2] + (argv[3] << 7);
        if (argc == 36) {
          for (uint8_t i = 0; i < 16; i++) {
            buffer[i] = argv[4 + 2*i] + (argv[5 + 2*i] << 7);
          }
          setupRadio(networkID, nodeID, buffer);
        } else setupRadio(networkID, nodeID);
      }
      break;
    case USR_CMD_RF_DATA:
      if (argc >= 2) {
        uint8_t nodeID = argv[0] + (argv[1] << 7);
        uint8_t datalen = (argc - 2) / 2;
        for (uint8_t i = 0; i < datalen; i++) {
          buffer[i] = argv[2 + 2*i] + (argv[3 + 2*i] << 7);
        }
        radio.sendWithRetry(nodeID, buffer, datalen, RFM69_RETRIES, RFM69_RETRY_DELAY);
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


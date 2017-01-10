#include <Boards.h>
#include <Firmata.h>

#include "UserSysex.h"
#include "RF.h"

char buffer[MAX_DATA_BYTES ];

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
    case USR_CMD_RF:
      encodeToBuffer(argc, argv);
      if (!rfSend((uint8_t*)buffer)) {
        Firmata.sendString("Unable to send data via Radio");
      }
      break;
    case 0x03:
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


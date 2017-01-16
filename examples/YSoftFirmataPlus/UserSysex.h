// User Sysex Commands

#define USR_CMD_ECHO      0x01 // Send the same data back with the same sysex command

#define USR_CMD_RF_CONFIG 0x02
// Configuration of networkID, nodeID and encrypt passphrase
// Password is optional, but when used, it must be 16 bytes long
// 0  START_SYSEX        (0xF0)
// 1  USR_CMD_RF_CONFIG  (0x02)
// 2  networkID LSB
// 3  networkID MSB
// 4  nodeID LSB
// 5  nodeID MSB
// 6  (optional) first password char LSB
// 7  (optional) first password char MSB
// ... additional password chars at positions 8-37
// 6/37 END_SYSEX (0xF7)

#define USR_CMD_RF_DATA   0x03
// Data to send or received data
// (when sending data nodeID is target, when receiving data nodeID is source)
// 0  START_SYSEX        (0xF0)
// 1  USR_CMD_RF_DATA    (0x03)
// 2  nodeID LSB
// 3  nodeID MSB
// 4  RSSI LSB (only used for received message)
// 5  RSSI MSB (only used for received message)
// 4/6 first byte LSB
// 5/7 first byte MSB
// ... additional bytes
// N  END_SYSEX (0xF7)

#define USR_CMD_4         0x04 // User command
#define USR_CMD_5         0x05 // User command
#define USR_CMD_6         0x06 // User command
#define USR_CMD_7         0x07 // User command
#define USR_CMD_8         0x08 // User command
#define USR_CMD_9         0x09 // User command
#define USR_CMD_A         0x0A // User command
#define USR_CMD_B         0x0B // User command
#define USR_CMD_C         0x0C // User command
#define USR_CMD_D         0x0D // User command
#define USR_CMD_E         0x0E // User command
#define USR_CMD_F         0x0F // User command


void userSysex(byte command, byte argc, byte *argv);

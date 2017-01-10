
// User Sysex Commands

#define USR_CMD_ECHO      0x01 // Send the same data back with the same sysex command
#define USR_CMD_RF        0x02 // Radio transmition
#define USR_CMD_3         0x03 // User command
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

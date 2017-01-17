#include <Boards.h>
#include <Firmata.h>

#include "YiPlayRF.h"
#include "UserSysex.h"

RFM69 radio(RFM69_NSS, RFM69_INTERRUPT_PIN, IS_RFM69HW, RFM69_INTERRUPT_NUM);
bool radioStarted = false;

byte packetBuffer[MAX_DATA_BYTES];

void setupRadio(uint8_t networkID, uint8_t nodeID, const char *encryptkey) {
	radio.initialize(FREQUENCY, nodeID, networkID);
	if (IS_RFM69HW) radio.setHighPower();
	if (encryptkey != NULL) radio.encrypt(encryptkey);
	radioStarted = true;
}

void radioListen() {
	if (!radioStarted || !radio.receiveDone()) return;

	int len = strlen((char*)radio.DATA);
	packetBuffer[0] = radio.SENDERID;
	packetBuffer[1] = radio.RSSI;
	memcpy(packetBuffer + 2, (byte*)radio.DATA, len);

	if (radio.ACKRequested()) radio.sendACK();

	Firmata.sendSysex(USR_CMD_RF_DATA, len + 2, packetBuffer);
}

#include "pch.h"
#include "ServerPacketHandler.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
	cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
	return false;
}
bool Handle_C_WELCOME(shared_ptr<PBSession> sessionRef, Protocol::C_Welcome& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	cout << "C_Welcome 패킷을 받았다." << endl;

	string encryptedStr = pkt.aeskey();
	vector<unsigned char> encryptedKey(encryptedStr.begin(), encryptedStr.end());
	vector<unsigned char> AESKey = RSAKeyManager::Decrypt(playerSessionRef->GetRSAKey(), encryptedKey);
	playerSessionRef->SetAESKey(AESKey);

	return true;
}
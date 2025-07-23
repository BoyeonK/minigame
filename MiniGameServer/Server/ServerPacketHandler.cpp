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
bool Handle_C_WELCOME(shared_ptr<PBSession> sessionRef, Protocol::C_Welcome& recvPkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	cout << "C_Welcome ��Ŷ�� �޾Ҵ�." << endl;

	string encryptedStr = recvPkt.aeskey();
	vector<unsigned char> encryptedKey(encryptedStr.begin(), encryptedStr.end());
	vector<unsigned char> AESKey = CryptoManager::Decrypt(playerSessionRef->GetRSAKey(), encryptedKey);

	if (AESKey.empty()) {
		//TODO : �������� AESKey�� Ȯ������ ������ ��� ����ó��
		cout << "AES Key ��ȣȭ ����!" << endl;
		playerSessionRef->Disconnect();
		return false;
	}

	playerSessionRef->SetAESKey(move(AESKey));

	Protocol::S_WelcomeResponse sendPkt;
	sendPkt.set_message(recvPkt.message());
	sendPkt.set_success(true);
	//shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeSendBufferRef()

	return true;
}
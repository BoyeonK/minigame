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

bool Handle_C_ENCRYPTED(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Encrypted& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	//TODO : Session에 저장된 AESKey를 통해서 원본 Protobuf로 복구한 후, Handler함수를 동작시킨다.
	return false;
}

bool Handle_C_WELCOME(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Welcome& recvPkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	cout << "C_Welcome 패킷을 받았다." << endl;

	string encryptedStr = recvPkt.aeskey();
	vector<unsigned char> encryptedKey(encryptedStr.begin(), encryptedStr.end());
	vector<unsigned char> AESKey = CryptoManager::Decrypt(playerSessionRef->GetRSAKey(), encryptedKey);

	if (AESKey.empty()) {
		//TODO : 정상적인 AESKey를 확보하지 못했을 경우 예외처리
		cout << "AES Key 복호화 실패!" << endl;
		playerSessionRef->Disconnect();
		return false;
	}

	playerSessionRef->SetAESKey(move(AESKey));

	S2C_Protocol::S_WelcomeResponse sendPkt;
	sendPkt.set_message(recvPkt.message());
	sendPkt.set_success(true);
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeSendBufferRef(sendPkt, playerSessionRef->GetAESKey());
	if (sendBuffer == nullptr) {
		return false;
	}
	playerSessionRef->Send(sendBuffer);
	cout << "그 뭐더라 그거 전송 완료" << endl;
	return true;
}

bool Handle_C_LOGIN(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Login& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	//TODO : ID와 password를 받아서 로그인을 수행한다.
	return false;
}

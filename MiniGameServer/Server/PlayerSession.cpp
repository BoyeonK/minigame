#include "pch.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"
#include "ServerPacketHandler.h"

void PlayerSession::OnConnected() {
	cout << "Player Session Onconnected!" << endl;

	EVP_PKEY* rawKey = GCryptoManager->PopKey();
	if (!rawKey) {
		cout << "Failed to Pop RSAKey!" << endl;
		Disconnect();
		return;
	}
	vector<unsigned char> publicKey = GCryptoManager->ExtractPublicKey(rawKey);
	_RSAKey = EVP_PKEY_dup(rawKey);
	GCryptoManager->ReturnKey(rawKey);
	if (!_RSAKey) {
		cout << "Failed to duplicate RSAKey!" << endl;
		Disconnect();
		return;
	}

	Protocol::S_Welcome sendPkt;
	sendPkt.set_publickey(publicKey.data(), publicKey.size());
	sendPkt.set_gameversion(_gameVersion);

	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeSendBufferRef(sendPkt);
	Send(sendBuffer);
}

void PlayerSession::OnDisconnected() {
	cout << "Player Session OnDisconnected!" << endl;
}

void PlayerSession::OnRecvPacket(unsigned char* buffer, int32_t len) {
	ServerPacketHandler::HandlePacket(static_pointer_cast<PBSession>(shared_from_this()), buffer, len);
}

EVP_PKEY* PlayerSession::GetRSAKey() {
	return _RSAKey;
}

void PlayerSession::SetAESKey(vector<unsigned char>&& AESKey) {
	_AESKey = std::move(AESKey);

	//디버그용. 추후 삭제 예정.
	cout << "SetAESKey" << endl;
}

vector<unsigned char> PlayerSession::GetAESKey() {
	return _AESKey;
}

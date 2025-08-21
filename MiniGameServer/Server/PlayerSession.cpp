#include "pch.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"
#include "S2CPacketMaker.h"
#include "S2CPacketHandler.h"

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

	S2C_Protocol::S_Welcome sendPkt = S2CPacketMaker::MakeSWelcome(publicKey, _gameVersion);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(sendPkt);
	Send(sendBuffer);
}

void PlayerSession::OnDisconnected() {
	cout << "Player Session OnDisconnected!" << endl;
}

void PlayerSession::OnRecvPacket(unsigned char* buffer, int32_t len) {
	S2CPacketHandler::HandlePacket(static_pointer_cast<PBSession>(shared_from_this()), buffer, len);
}

EVP_PKEY* PlayerSession::GetRSAKey() {
	return _RSAKey;
}

void PlayerSession::SetAESKey(vector<unsigned char>&& AESKey) {
	_AESKey = move(AESKey);

	//디버그용. 추후 삭제 예정.
	cout << "SetAESKey (Move)" << endl;
}

void PlayerSession::SetAESKey(vector<unsigned char>& AESKey) {
	_AESKey = AESKey;

	//디버그용. 추후 삭제 예정.
	cout << "SetAESKey (Copy)" << endl;
}

vector<unsigned char> PlayerSession::GetAESKey() {
	return _AESKey;
}

void PlayerSession::SetSecureLevel(int32_t lv) {
	_secureLevel = lv;
}

void PlayerSession::SetDbid(int32_t dbid) {
	_dbid = dbid;
}

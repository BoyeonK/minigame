#include "pch.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"
#include "ServerPacketHandler.h"

void PlayerSession::OnConnected() {
	cout << "Player Session Onconnected!" << endl;

	EVP_PKEY* rawKey = RSAManager->PopKey();
	if (!rawKey) {
		cout << "Failed to Pop RSAKey!" << endl;
		Disconnect();
		return;
	}
	vector<unsigned char> publicKey = RSAManager->ExtractPublicKey(rawKey);
	_RSAKey = EVP_PKEY_dup(rawKey);
	RSAManager->ReturnKey(rawKey);
	if (!_RSAKey) {
		cout << "Failed to duplicate RSAKey!" << endl;
		Disconnect();
		return;
	}

	Protocol::S_Welcome pkt;
	pkt.set_publickey(publicKey.data(), publicKey.size());
	pkt.set_gameversion(_gameVersion);

	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeSendBufferRef(pkt);
	Send(sendBuffer);
}

void PlayerSession::OnDisconnected() {
	cout << "Player Session OnDisconnected!" << endl;
}

void PlayerSession::OnRecvPacket(unsigned char* buffer, int32_t len) {
	ServerPacketHandler::HandlePacket(static_pointer_cast<PBSession>(shared_from_this()), buffer, len);
}

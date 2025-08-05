#include "pch.h"
#include "ServerPacketHandler.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"
#include "ServerPacketMaker.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
	cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
	return false;
}

bool Handle_C_ENCRYPTED(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Encrypted& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	//TODO : Session�� ����� AESKey�� ���ؼ� ���� Protobuf�� ������ ��, Handler�Լ��� ���۽�Ų��.
	const string& iv_str = pkt.iv();
	const string& ciphertext_str = pkt.ciphertext();
	const string& tag_str = pkt.tag();
	int32_t msgId = pkt.msgid();
	vector<unsigned char> aesKey = playerSessionRef->GetAESKey();

	vector<unsigned char> aad(sizeof(msgId));
	memcpy(aad.data(), &msgId, sizeof(msgId));

	auto ctx_deleter = [](EVP_CIPHER_CTX* ctx) { EVP_CIPHER_CTX_free(ctx); };
	unique_ptr<EVP_CIPHER_CTX, decltype(ctx_deleter)> ctx(EVP_CIPHER_CTX_new(), ctx_deleter);

	if (!ctx) {
		std::cerr << "Error: EVP_CIPHER_CTX_new failed." << std::endl;
		return false;
	}

	if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
		std::cerr << "Error: EVP_DecryptInit_ex (cipher) failed." << std::endl;
		return false;
	}

	if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv_str.length(), NULL) != 1) {
		std::cerr << "Error: EVP_CIPHER_CTX_ctrl (IV length) failed." << std::endl;
		return false;
	}

	if (EVP_DecryptInit_ex(ctx.get(), NULL, NULL, aesKey.data(), (const unsigned char*)iv_str.c_str()) != 1) {
		std::cerr << "Error: EVP_DecryptInit_ex (key, IV) failed." << std::endl;
		return false;
	}

	int len = 0;
	if (EVP_DecryptUpdate(ctx.get(), NULL, &len, aad.data(), aad.size()) != 1) {
		std::cerr << "Error: EVP_DecryptUpdate (AAD) failed." << std::endl;
		return false;
	}

	std::vector<unsigned char> plaintext(ciphertext_str.length());
	if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, (const unsigned char*)ciphertext_str.c_str(), ciphertext_str.length()) != 1) {
		std::cerr << "Error: EVP_DecryptUpdate (ciphertext) failed." << std::endl;
		return false;
	}
	int plaintext_len = len;

	if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, tag_str.length(), (void*)tag_str.c_str()) != 1) {
		std::cerr << "Error: EVP_CIPHER_CTX_ctrl (set tag) failed." << std::endl;
		return false;
	}

	if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
		std::cerr << "Error: EVP_DecryptFinal_ex failed. Tag verification failed." << std::endl;
		// ���� ���д� �ſ� �߿��� ���� �̺�Ʈ�Դϴ�. �α׸� ����� ���� �����ϴ�.
		return false;
	}
	plaintext_len += len;
	plaintext.resize(plaintext_len);

	return true;
}

bool Handle_C_WELCOME(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Welcome& recvPkt) {
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

	S2C_Protocol::S_WelcomeResponse sendPkt = ServerPacketMaker::MakeSWelcomeResponse(recvPkt.message(), true);
	//shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeSendBufferRef(sendPkt);
	shared_ptr<SendBuffer> sendBuffer = ServerPacketHandler::MakeSendBufferRef(sendPkt, playerSessionRef->GetAESKey());
	if (sendBuffer == nullptr) {
		return false;
	}
	playerSessionRef->Send(sendBuffer);
	cout << "S_WelcomeResponse ����" << endl;
	return true;
}

bool Handle_C_LOGIN(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Login& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	//TODO : ID�� password�� �޾Ƽ� �α����� �����Ѵ�.
	cout << "��" << endl;

	return false;
}

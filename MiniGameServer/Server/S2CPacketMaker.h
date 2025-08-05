#pragma once
#include "ServerGlobal.h"

class S2CPacketMaker {
public:
	template<typename PBType>
	static S2C_Protocol::S_Encrypted MakeSEncrypted(const PBType& pkt, uint16_t pktId, const vector<unsigned char>& AESKey) {
		S2C_Protocol::S_Encrypted pkt;

		string serializedStr = pkt.SerializeAsString();
		vector<unsigned char> plaintext(serializedStr.begin(), serializedStr.end());
		vector<unsigned char> iv, ciphertext, tag;
		if (!(GCryptoManager->Encrypt(AESKey, plaintext, (int32_t)pktId, iv, ciphertext, tag))) {
			cout << "복호화 실패" << endl;
			return pkt;
		}
		pkt.set_iv(iv.data(), iv.size());
		pkt.set_ciphertext(ciphertext.data(), ciphertext.size());
		pkt.set_tag(tag.data(), tag.size());
		pkt.set_msgid(pktId);
		return pkt;
	}

	static S2C_Protocol::S_Welcome MakeSWelcome(const vector<unsigned char>& publicKey, const int32_t gameVersion) {
		S2C_Protocol::S_Welcome pkt;
		if (!publicKey.empty()) 
			pkt.set_publickey(publicKey.data(), publicKey.size());
		pkt.set_gameversion(gameVersion);
		return pkt;
	}

	static S2C_Protocol::S_WelcomeResponse MakeSWelcomeResponse(const string& message, const bool success) {
		S2C_Protocol::S_WelcomeResponse pkt;
		pkt.set_message(message);
		pkt.set_success(success);
		return pkt;
	}

	static S2C_Protocol::S_Login MakeSLogin() {
		S2C_Protocol::S_Login pkt;
		return pkt;
	}
};

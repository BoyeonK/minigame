#pragma once
#include "ServerGlobal.h"
#include "S2C_Protocol.pb.h"

class S2CPacketMaker {
public:
	template<typename PBType>
	static S2C_Protocol::S_Encrypted MakeSEncrypted(const PBType& basePkt, const uint16_t& pktId, const vector<unsigned char>& AESKey) {
		S2C_Protocol::S_Encrypted pkt;

		string serializedStr = basePkt.SerializeAsString();
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

	static S2C_Protocol::S_Welcome MakeSWelcome(const vector<unsigned char>& publicKey, const int32_t& gameVersion) {
		S2C_Protocol::S_Welcome pkt;
		if (!publicKey.empty()) 
			pkt.set_publickey(publicKey.data(), publicKey.size());
		pkt.set_gameversion(gameVersion);
		return pkt;
	}

	static S2C_Protocol::S_WelcomeResponse MakeSWelcomeResponse(const string& message, const bool& success) {
		S2C_Protocol::S_WelcomeResponse pkt;
		pkt.set_message(message);
		pkt.set_success(success);
		return pkt;
	}

	static S2C_Protocol::S_Login MakeSLogin(const int32_t& dbid) {
		S2C_Protocol::S_Login pkt;
		pkt.set_dbid(dbid);
		return pkt;
	}

	static S2C_Protocol::S_Login MakeSLogin(const bool& incorrect_id) {
		S2C_Protocol::S_Login pkt;
		if (incorrect_id) {
			string err = "asdf";
			pkt.set_err(err);
		}
		else {
			string err = "qwer";
			pkt.set_err(err);
		}
		return pkt;
	}

	static S2C_Protocol::S_Logout MakeSLogout(const bool& isSucceed) {
		S2C_Protocol::S_Logout pkt;
		pkt.set_success(isSucceed);
		return pkt;
	}

	static S2C_Protocol::S_MatchmakeKeepAlive MakeSMatchmakeKeepAlive(const int32_t& gameId) {
		S2C_Protocol::S_MatchmakeKeepAlive pkt;
		pkt.set_gameid(gameId);
		pkt.set_senttimetick(::GetTickCount64());
		return pkt;
	}

	static S2C_Protocol::S_ExcludedFromMatch MakeSExcludedFromMatch(const bool& isUserRequest) {
		S2C_Protocol::S_ExcludedFromMatch pkt;
		pkt.set_isuserrequest(isUserRequest);
		return pkt;
	}
};

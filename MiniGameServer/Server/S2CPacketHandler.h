#pragma once
#include "S2C_Protocol.pb.h"
#include "ServerGlobal.h"

using PacketHandlerFunc = function<bool(shared_ptr<PBSession>, unsigned char*, int32_t)>;
using PlaintextHandlerFunc = function<bool(shared_ptr<PBSession>, vector<unsigned char>&)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];
extern PlaintextHandlerFunc PlaintextHandler[UINT16_MAX];

//name convention : 서버에서 보내는(클라가 받는) S_
//					클라에서 보내는(서버가 받는) C_
enum : uint16_t {
	PKT_S_WELCOME = 0,
	PKT_C_WELCOME = 1,
	PKT_S_WELCOME_RESPONSE = 2,
	PKT_S_ENCRYPTED = 3,
	PKT_C_ENCRYPTED = 4,
	PKT_C_LOGIN = 5,
	PKT_S_LOGIN = 6,
	PKT_C_CREATE_ACCOUNT = 7,
	PKT_S_CREATE_ACCOUNT = 8,
	PKT_C_LOGOUT = 9,
	PKT_S_LOGOUT = 10,
};

bool Handle_Invalid(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len);
bool Handle_C_Encrypted(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Encrypted& pkt);
bool Handle_C_Welcome(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Welcome& pkt);
bool Handle_C_Login(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Login& pkt);
bool Handle_C_CreateAccount(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_CreateAccount& pkt);
bool Handle_C_Logout(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Logout& pkt);

class S2CPacketHandler {
public:
	static void Init() {
		for (int32_t i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_Invalid;
		
		//클라이언트로부터 받은 바이너리에서 헤더를 제외한 나머지 부분을 알맞은 protobuf타입으로 캐스팅하고, 알맞는 핸들러 함수를 호출.
		GPacketHandler[PKT_C_ENCRYPTED] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Encrypted>(Handle_C_Encrypted, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Welcome>(Handle_C_Welcome, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Login>(Handle_C_Login, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_CREATE_ACCOUNT] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_CreateAccount>(Handle_C_CreateAccount, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_LOGOUT] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Logout>(Handle_C_Logout, sessionRef, buffer, len); };

		//C_Encrypted를 복호화하여 얻은 바이너리를 알맞은 protobuf타입으로 캐스팅하고, 알맞은 핸들러 함수를 호출.
		PlaintextHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Welcome>(Handle_C_Welcome, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_LOGIN] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Login>(Handle_C_Login, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_CREATE_ACCOUNT] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_CreateAccount>(Handle_C_CreateAccount, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_LOGOUT] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Logout>(Handle_C_Logout, sessionRef, plaintext); };
	}

	static bool HandlePacket(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
		//TODO: Session에 허락된 범주의 pktId의 핸들러만 실행하기.
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		if (header->_id > sessionRef->GetSecureLevel()) {
			cout << "보안 레벨에 맞지 않는 패킷. " << endl;
			return false;
		}

		return GPacketHandler[header->_id](sessionRef, buffer, len);
	}
	//SendBufferChunk에서 SendBuffer를 할당받을 함수들.
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Encrypted& pkt) { return MakeSendBufferRef(pkt, PKT_S_ENCRYPTED); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Welcome& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOME); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Welcome& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOME, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_WelcomeResponse& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOME_RESPONSE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_WelcomeResponse& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOME_RESPONSE, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Login& pkt) { return MakeSendBufferRef(pkt, PKT_S_LOGIN); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Login& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_LOGIN, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_CreateAccount& pkt) { return MakeSendBufferRef(pkt, PKT_S_CREATE_ACCOUNT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_CreateAccount& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_CREATE_ACCOUNT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Logout& pkt) { return MakeSendBufferRef(pkt, PKT_S_LOGOUT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Logout& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_LOGOUT, AESKey); }

private:
	template<typename PBType, typename HandlerFunc>
	static bool HandlePacket(HandlerFunc func, shared_ptr<PBSession>& sessionRef, unsigned char* buffer, int32_t len) {
		PBType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;
		return func(sessionRef, pkt);
	}

	template<typename PBType, typename HandlerFunc>
	static bool HandlePlaintext(HandlerFunc func, shared_ptr<PBSession>& sessionRef, vector<unsigned char> plaintext) {
		PBType pkt;
		if (pkt.ParseFromArray(plaintext.data(), plaintext.size()) == false)
			return false;
		return func(sessionRef, pkt);
	}

	//PB로 직렬화된 패킷을 Local SendBufferChunk에 로드하고
	//해당 SendBuffer를 Return
	template<typename PBType>
	static shared_ptr<SendBuffer> MakeSendBufferRef(const PBType& pkt, uint16_t pktId) {
		uint16_t dataSize = static_cast<uint16_t>(pkt.ByteSizeLong());
		uint16_t packetSize = dataSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBufferRef = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBufferRef->Buffer());
		header->_size = packetSize;
		header->_id = pktId;
		pkt.SerializeToArray(&header[1]/*(++header)*/, dataSize);
		sendBufferRef->Close(packetSize);

		return sendBufferRef;
	}

	//PB로 직렬화된 패킷을 AES키를 이용하여 암호화, SendBufferChunk에 로드하고 해당 SendBuffer를 Return
	template<typename PBType>
	static shared_ptr<SendBuffer> MakeSendBufferRef(const PBType& pkt, uint16_t pktId, const vector<unsigned char>& AESKey) {
		string serializedStr = pkt.SerializeAsString();
		vector<unsigned char> plaintext(serializedStr.begin(), serializedStr.end());
		vector<unsigned char> iv, ciphertext, tag;
		if (!(GCryptoManager->Encrypt(AESKey, plaintext, (int32_t)pktId, iv, ciphertext, tag))) {
			cout << "복호화 실패" << endl;
			return nullptr;
		}
		S2C_Protocol::S_Encrypted sendPkt;
		sendPkt.set_iv(iv.data(), iv.size());
		sendPkt.set_ciphertext(ciphertext.data(), ciphertext.size());
		sendPkt.set_tag(tag.data(), tag.size());
		sendPkt.set_msgid(pktId);

		uint16_t dataSize = static_cast<uint16_t>(sendPkt.ByteSizeLong());
		uint16_t packetSize = dataSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBufferRef = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBufferRef->Buffer());
		header->_size = packetSize;
		header->_id = PKT_S_ENCRYPTED;
		sendPkt.SerializeToArray(&header[1]/*(++header)*/, dataSize);
		sendBufferRef->Close(packetSize);

		return sendBufferRef;
	}
};
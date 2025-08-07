#pragma once
#include "S2C_Protocol.pb.h"
#include "ServerGlobal.h"

using PacketHandlerFunc = function<bool(shared_ptr<PBSession>, unsigned char*, int32_t)>;
using PlaintextHandlerFunc = function<bool(shared_ptr<PBSession>, vector<unsigned char>&)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];
extern PlaintextHandlerFunc PlaintextHandler[UINT16_MAX];

//name convention : �������� ������(Ŭ�� �޴�) S_
//					Ŭ�󿡼� ������(������ �޴�) C_
enum : uint16_t {
	PKT_S_ENCRYPTED = 0,
	PKT_C_ENCRYPTED = 1,
	PKT_S_WELCOME = 2,
	PKT_C_WELCOME = 3,
	PKT_S_WELCOMERESPONSE = 4,
	PKT_C_LOGIN = 5,
	PKT_S_LOGIN = 6,
};

bool Handle_INVALID(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len);
bool Handle_C_ENCRYPTED(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Encrypted& pkt);
bool Handle_C_WELCOME(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Welcome& pkt);
bool Handle_C_LOGIN(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Login& pkt);

class ServerPacketHandler {
public:
	static void Init() {
		for (int32_t i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		
		GPacketHandler[PKT_C_ENCRYPTED] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Encrypted>(Handle_C_ENCRYPTED, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Welcome>(Handle_C_WELCOME, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Login>(Handle_C_LOGIN, sessionRef, buffer, len); };

		PlaintextHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Welcome>(Handle_C_WELCOME, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_LOGIN] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Login>(Handle_C_LOGIN, sessionRef, plaintext); };
	}

	static bool HandlePacket(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
		//TODO: Session�� ����� ������ pktId�� �ڵ鷯�� �����ϱ�.
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->_id](sessionRef, buffer, len);
	}
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Encrypted& pkt) { return MakeSendBufferRef(pkt, PKT_S_ENCRYPTED); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Welcome& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOME); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Welcome& pkt, const vector<unsigned char> AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOME, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_WelcomeResponse& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOMERESPONSE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_WelcomeResponse& pkt, const vector<unsigned char> AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOMERESPONSE, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Login& pkt) { return MakeSendBufferRef(pkt, PKT_S_LOGIN); }

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

	//PB�� ����ȭ�� ��Ŷ�� Local SendBufferChunk�� �ε��ϰ�
	//�ش� SendBuffer�� Return
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

	//PB�� ����ȭ�� ��Ŷ�� AESŰ�� �̿��Ͽ� ��ȣȭ, SendBufferChunk�� �ε��ϰ� �ش� SendBuffer�� Return
	template<typename PBType>
	static shared_ptr<SendBuffer> MakeSendBufferRef(const PBType& pkt, uint16_t pktId, const vector<unsigned char>& AESKey) {
		string serializedStr = pkt.SerializeAsString();
		vector<unsigned char> plaintext(serializedStr.begin(), serializedStr.end());
		vector<unsigned char> iv, ciphertext, tag;
		if (!(GCryptoManager->Encrypt(AESKey, plaintext, (int32_t)pktId, iv, ciphertext, tag))) {
			cout << "��ȣȭ ����" << endl;
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
		header->_id = 0;
		sendPkt.SerializeToArray(&header[1]/*(++header)*/, dataSize);
		sendBufferRef->Close(packetSize);

		return sendBufferRef;
	}
};
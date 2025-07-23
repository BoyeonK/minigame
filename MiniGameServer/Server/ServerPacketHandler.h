#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = function<bool(shared_ptr<PBSession>, unsigned char*, int32_t)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

//name convention : 서버에서 보내는(클라가 받는) S_
//					클라에서 보내는(서버가 받는) C_
enum : uint16_t {
	PKT_S_ENCRYPTED = 0,
	PKT_C_ENCRYPTED = 1,
	PKT_S_WELCOME = 2,
	PKT_C_WELCOME = 3,
	PKT_S_WELCOMERESPONSE = 4,
};

bool Handle_INVALID(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len);
bool Handle_C_WELCOME(shared_ptr<PBSession> sessionRef, Protocol::C_Welcome& pkt);

class ServerPacketHandler {
public:
	static void Init() {
		for (int32_t i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		
		GPacketHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<Protocol::C_Welcome>(Handle_C_WELCOME, sessionRef, buffer, len); };
	}

	static bool HandlePacket(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->_id](sessionRef, buffer, len);
	}
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_Welcome& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOME); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_Welcome& pkt, const vector<unsigned char> AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOME, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_WelcomeResponse& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOMERESPONSE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_WelcomeResponse& pkt, const vector<unsigned char> AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOMERESPONSE, AESKey); }

private:
	template<typename PBType, typename HandlerFunc>
	static bool HandlePacket(HandlerFunc func, shared_ptr<PBSession>& sessionRef, unsigned char* buffer, int32_t len) {
		PBType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
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
		CryptoManager::Encrypt(AESKey, plaintext, iv, ciphertext, tag);
		Protocol::S_Encrypted sendPkt;
		sendPkt.set_iv(iv.data(), iv.size());
		sendPkt.set_ciphertext(ciphertext.data(), ciphertext.size());
		sendPkt.set_tag(tag.data(), tag.size());

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
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
	/*
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_EnterGame& pkt) { return MakeSendBufferRef(pkt, PKT_S_ENTER_GAME); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_LeaveGame& pkt) { return MakeSendBufferRef(pkt, PKT_S_LEAVE_GAME); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_Despawn& pkt) { return MakeSendBufferRef(pkt, PKT_S_DESPAWN); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_Spawn& pkt) { return MakeSendBufferRef(pkt, PKT_S_SPAWN); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_Move& pkt) { return MakeSendBufferRef(pkt, PKT_S_MOVE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const Protocol::S_Skill& pkt) { return MakeSendBufferRef(pkt, PKT_S_SKILL); }
	*/

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

	//PB로 직렬화된 패킷을 AES키를 이용하여 암호화, SendBufferChunk에 로드하고 해당 SendBuffer를 Return 할 예정
	//서버에서 암호화 한 경우, PacketHeader의 packetID부분을 0으로 통일하여 packetID를 숨김.
	template<typename PBType>
	static shared_ptr<SendBuffer> MakeSendBufferRef(const PBType& pkt, uint16_t pktId, int AES) {
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
};
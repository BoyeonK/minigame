#include "pch.h"
#include "ServerPacketHandler.h"
#include "PlayerSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
	cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
	return false;
}
/*
bool Handle_C_MOVE(shared_ptr<PBSession> sessionRef, Protocol::C_Move& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());

	return true;
}

bool Handle_C_SKILL(shared_ptr<PBSession> sessionRef, Protocol::C_Skill& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());

	return true;
}
*/
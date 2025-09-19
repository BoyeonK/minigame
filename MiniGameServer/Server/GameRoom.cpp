#include "pch.h"
#include "GameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void GameRoom::SetRoomId(int32_t roomId) {
	_roomId = roomId;
}

int32_t GameRoom::GetRoomId() const {
	return _roomId;
}

GameRoom::GameState GameRoom::GetState() const {
	return _state;
}

int32_t GameRoom::GenerateUniqueGameObjectId() {
	return _nxtObjectId++;
}

void GameRoom::RegisterGameObject(const shared_ptr<UnityGameObject>& obj) {
	_vecGameObjects.push_back(obj);
	_hmGameObjects.insert({ obj->GetObjectId(), obj});
}

void GameRoom::BroadCast(shared_ptr<SendBuffer> sendBuffer) {
	if (sendBuffer == nullptr)
		return;
	for (auto& playerWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerRef = playerWRef.lock();
		if (playerRef == nullptr)
			continue;
		playerRef->Send(sendBuffer);
	}
}

void GameRoom::BroadCastSpawn(const shared_ptr<UnityGameObject>& objRef) {
	if (objRef == nullptr)
		return;
	S2C_Protocol::S_SpawnGameObject pkt = S2CPacketMaker::MakeSSpawnGameObject(objRef);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void GameRoom::BroadCastDespawn(const shared_ptr<UnityGameObject>& objRef) {
	if (objRef == nullptr)
		return;
	S2C_Protocol::S_SpawnGameObject pkt = S2CPacketMaker::MakeSSpawnGameObject(objRef);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}


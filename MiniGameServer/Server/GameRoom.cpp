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

void GameRoom::RegisterGameObject(shared_ptr<UnityGameObject> obj) {
	_vecGameObjects.push_back(obj);
	//_hmGameObjects.insert()
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

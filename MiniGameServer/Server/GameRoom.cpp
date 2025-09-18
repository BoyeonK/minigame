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
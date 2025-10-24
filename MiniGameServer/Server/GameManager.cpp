#include "pch.h"
#include "GameManager.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "TestGameRoom.h"

int32_t GameManager::GetPublicRecord() {
	return _publicRecord;
}

void GameManager::AddRoom(shared_ptr<GameRoom> room) {
	unique_lock<shared_mutex> lock(_roomsLock);
	_rooms.push_back(room);
}

void GameManager::RemoveInvalidRoom() {
	if (::GetTickCount64() - _lastRemoveRoomTick > 5000) {
		unique_lock<shared_mutex> lock(_roomsLock);
		_lastRemoveRoomTick = ::GetTickCount64();
		auto new_end = remove_if(_rooms.begin(), _rooms.end(),
			[](const shared_ptr<GameRoom>& gameRoomRef) {
				return (gameRoomRef->GetState() == GameRoom::GameState::EndGame);
			});

		if (new_end != _rooms.end())
			cout << "Invalid Room Cleared" << endl;

		_rooms.erase(new_end, _rooms.end());
	}
}
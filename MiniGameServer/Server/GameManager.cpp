#include "pch.h"
#include "GameManager.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "RaceRoom.h"

int32_t GameManager::GetPublicRecord() {
	lock_guard<mutex> lock(_recordLock);
	return _publicRecord;
}

string GameManager::GetPublicRecorder() {
	lock_guard<mutex> lock(_recordLock);
	return _publicRecorder;
}

void GameManager::SetPublicRecord(string playerId, int32_t record) {
	lock_guard<mutex> lock(_recordLock);
	_publicRecorder = playerId;
	_publicRecord = record;
}

void GameManager::AddRoom(shared_ptr<GameRoom> room) {
	unique_lock<mutex> lock(_roomsToBeAddedLock);
	_roomsToBeAdded.push_back(room);
}

void GameManager::AddRoomsFromPendingVector() {
	vector<shared_ptr<GameRoom>> roomsToBeAddedCopy;

	{
		unique_lock<mutex> lock(_roomsToBeAddedLock);
		roomsToBeAddedCopy = move(_roomsToBeAdded);
	}

	{
		unique_lock<mutex> lock(_roomsLock);
		for (auto& roomRef : roomsToBeAddedCopy)
			_rooms.push_back(roomRef);
		_roomCount = _rooms.size();
	}
}

void GameManager::RemoveInvalidRoom() {
	if (::GetTickCount64() - _lastRemoveRoomTick > _removeRoomTickPeriod) {
		unique_lock<mutex> lock(_roomsLock);
		_lastRemoveRoomTick = ::GetTickCount64();
		auto new_end = remove_if(_rooms.begin(), _rooms.end(),
			[](const shared_ptr<GameRoom>& gameRoomRef) {
				return (gameRoomRef->GetState() == GameRoom::GameState::EndGame);
			});

		if (new_end != _rooms.end())
			cout << "Invalid Room Cleared" << endl;

		_rooms.erase(new_end, _rooms.end());
		_roomCount = _rooms.size();
	}
}
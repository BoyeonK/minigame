#include "pch.h"
#include "RaceManager.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "RaceRoom.h"

void RaceManager::Push(WatingPlayerData pd) {
	_matchQueue.Push(move(pd));
}

void RaceManager::Push(vector<WatingPlayerData> pdv) {
	_matchQueue.Push(move(pdv));
}

void RaceManager::RenewMatchQueue() {
	if (::GetTickCount64() - _lastRenewMatchQueueTick > 3000) {
		_lastRenewMatchQueueTick = ::GetTickCount64();
		_matchQueue.FlushTempQueueAndSort();
	}
}

void RaceManager::MatchMake() {
	vector<vector<WatingPlayerData>> pdvv = _matchQueue.SearchMatchGroups();
	for (auto& pdv : pdvv) {
		bool isReady = true;
		fill(_excluded.begin(), _excluded.end(), false);
		for (int i = 0; i < _quota; i++) {
			shared_ptr<PlayerSession> playerSessionRef = pdv[i].playerSessionWRef.lock();
			if (playerSessionRef == nullptr) {
				isReady = false;
				_excluded[i] = true;
			}
			else if (playerSessionRef->GetMatchingState() != GameType::Race) {
				isReady = false;
				_excluded[i] = true;

				S2C_Protocol::S_ExcludedFromMatch pkt = S2CPacketMaker::MakeSExcludedFromMatch(false);
				shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
				playerSessionRef->Send(sendBufferRef);
				playerSessionRef->SetMatchingState(GameType::None);
			}
		}

		if (isReady) {
			MakeRoom(move(pdv));
		}
		else {
			for (int i = 0; i < _quota; i++) {
				if (!_excluded[i])
					_matchQueue.Push(move(pdv[i]));
			}
		}
	}
}

void RaceManager::MakeRoom(vector<WatingPlayerData>&& pdv) {
	shared_ptr<RaceRoom> newRoomRef = { objectPool<RaceRoom>::alloc(), objectPool<RaceRoom>::dealloc };
	newRoomRef->SetRoomId(_nxtRoomId.fetch_add(1));
	AddRoom(newRoomRef);
	newRoomRef->PostEvent(&RaceRoom::Init, move(pdv));
}

bool RaceManager::RenewPublicRecordFromDB() {
	return DBManager->S2D_PublicRecord(int(_ty));
}

bool RaceManager::CompareAndRenewPublicRecord(int32_t dbid, int32_t score) {
	return false;
}

void RaceManager::Update() {
	uint64_t now = ::GetTickCount64();
	if (now - _lastUpdateRoomTick < _updateTickPeriod)
		return;
	_lastUpdateRoomTick = now;

	{
		lock_guard<mutex> lock(_roomsLock);
		for (auto& roomRef : _rooms) {
			roomRef->PostEvent(&GameRoom::Update);
		}
	}
}

#include "pch.h"
#include "MoleManager.h"
#include "S2CPacketMaker.h"
#include "S2CPacketHandler.h"
#include "MoleRoom.h"

void MoleManager::Push(WatingPlayerData pd) {
	_matchQueue.Push(move(pd));
}

void MoleManager::Push(vector<WatingPlayerData> pdv) {
	_matchQueue.Push(move(pdv));
}

void MoleManager::RenewMatchQueue() {
	if (::GetTickCount64() - _lastRenewMatchQueueTick > 3000) {
		_lastRenewMatchQueueTick = ::GetTickCount64();
		_matchQueue.FlushTempQueueAndSort();
	}
}

void MoleManager::MatchMake() {
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
			else if (playerSessionRef->GetMatchingState() != GameType::Mole) {
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

void MoleManager::MakeRoom(vector<WatingPlayerData>&& pdv) {
	shared_ptr<MoleRoom> newRoomRef = { objectPool<MoleRoom>::alloc(), objectPool<MoleRoom>::dealloc };
	newRoomRef->SetRoomId(_nxtRoomId.fetch_add(1));
	AddRoom(newRoomRef);
	newRoomRef->PostEvent(&MoleRoom::Init, move(pdv));
}

bool MoleManager::RenewPublicRecordFromDB() {
	return DBManager->S2D_PublicRecord(int(_ty));
}

bool MoleManager::CompareAndRenewPublicRecord(int32_t dbid, int32_t score) {
	if (score < _publicRecord)
		return false;

	cout << "Mole 최고 기록 갱신 요청 " << dbid << "의 최고 기록 " << score << endl;
	DBManager->S2D_UpdatePublicRecord(int(_ty), dbid, score);
	return true;
}

void MoleManager::Update() {
	uint64_t now = ::GetTickCount64();
	if (now - _lastUpdateRoomTick < _updateTickPeriod)
		return;
	_lastUpdateRoomTick = now;

	{
		lock_guard<shared_mutex> lock(_roomsLock);
		for (auto& roomRef : _rooms) {
			roomRef->PostEvent(&GameRoom::Update);
		}
	}
}

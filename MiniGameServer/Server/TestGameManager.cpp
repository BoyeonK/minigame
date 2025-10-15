#include "pch.h"
#include "TestGameManager.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "TestGameRoom.h"

void TestGameManager::Push(WatingPlayerData pd) {
	_matchQueue.Push(move(pd));
}

void TestGameManager::Push(vector<WatingPlayerData> pdv) {
	_matchQueue.Push(move(pdv));
}

void TestGameManager::RenewMatchQueue() {
	if (::GetTickCount64() - _lastRenewMatchQueueTick > 3000) {
		_lastRenewMatchQueueTick = ::GetTickCount64();
		_matchQueue.FlushTempQueueAndSort();
	}
}

void TestGameManager::MatchMake() {
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
			else if (playerSessionRef->GetMatchingState() != GameType::TestGame) {
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

void TestGameManager::MakeRoom(vector<WatingPlayerData>&& pdv) {
	shared_ptr<TestGameRoom> newRoomRef = { objectPool<TestGameRoom>::alloc(), objectPool<TestGameRoom>::dealloc };
	newRoomRef->SetRoomId(_nxtRoomId.fetch_add(1));
	AddRoom(newRoomRef);
	newRoomRef->PostEvent(&TestGameRoom::Init, move(pdv));
}

void TestGameManager::Update() {
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

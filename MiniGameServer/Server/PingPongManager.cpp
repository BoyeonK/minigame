#include "pch.h"
#include "PingPongManager.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "PingPongGameRoom.h"

void PingPongManager::Push(WatingPlayerData pd) {
	_matchQueue.Push(move(pd));
}

void PingPongManager::Push(vector<WatingPlayerData> pdv) {
	_matchQueue.Push(move(pdv));
}

void PingPongManager::RenewMatchQueue() {
	if (::GetTickCount64() - _lastRenewTick > 3000) {
		_lastRenewTick = ::GetTickCount64();
		_matchQueue.FlushTempQueueAndSort();
	}
}

void PingPongManager::MatchMake() {
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
			else if (playerSessionRef->GetMatchingState() != GameType::PingPong) {
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

void PingPongManager::MakeRoom(vector<WatingPlayerData>&& pdv) {
	shared_ptr<PingPongGameRoom> newRoomRef = { objectPool<PingPongGameRoom>::alloc(), objectPool<PingPongGameRoom>::dealloc };
	newRoomRef->SetRoomId(_nxtRoomId.fetch_add(1));
	AddRoom(newRoomRef);
	newRoomRef->PostEvent(&PingPongGameRoom::Init, move(pdv));
}

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
	if (::GetTickCount64() - _lastRenewMatchQueueTick > 3000) {
		_lastRenewMatchQueueTick = ::GetTickCount64();
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

bool PingPongManager::RenewPublicRecordFromDB() {
	return DBManager->S2D_PublicRecord(int(_ty));
}

bool PingPongManager::CompareAndRenewPublicRecord(int32_t dbid, int32_t score) {
	if (score < _publicRecord)
		return false;

	DBManager->S2D_UpdatePublicRecord(int(_ty), dbid, score);
	return true;
}

void PingPongManager::Update() {
	uint64_t now = ::GetTickCount64();
	if (now - _lastUpdateRoomTick < _updateTickPeriod)
		return;
	_lastUpdateRoomTick = now;

	if (now - _lastPendingRoomsVectorAddTick > _pendingRoomsVectorAddTickPeriod) {
		_lastPendingRoomsVectorAddTick = now;
		AddRoomsFromPendingVector();
	}

	{
		lock_guard<mutex> lock(_roomsLock);
		for (auto& roomRef : _rooms) {
			roomRef->PostEvent(&GameRoom::Update);
		}
	}
}

void PingPongManager::makeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed, S2C_Protocol::S_P_Bullet* pBullet) {
	S2C_Protocol::UnityGameObject* bullet_ptr = pBullet->mutable_bullet();
	bullet_ptr->set_objecttype(bulletType);
	S2C_Protocol::XYZ* pos_ptr = bullet_ptr->mutable_position();
	pos_ptr->set_x(px);
	pos_ptr->set_y(0.2f);
	pos_ptr->set_z(pz);

	S2C_Protocol::XYZ* moveDir_ptr = pBullet->mutable_movedir();
	moveDir_ptr->set_x(sx);
	moveDir_ptr->set_z(sz);

	pBullet->set_speed(speed);
	pBullet->set_lastcollider(-1);
}

float PingPongManager::GetToleranceRate() {
	return _toleranceRate;
}



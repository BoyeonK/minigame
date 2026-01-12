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

	cout << "pingpong 최고 기록 갱신 요청 " << dbid << "의 최고 기록 " << score << endl;
	DBManager->S2D_UpdatePublicRecord(int(_ty), dbid, score);
	return true;
}

void PingPongManager::Update() {
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

void PingPongManager::InitPattern() {
	easyPatterns.resize(6);
	mediumPatterns.resize(6);
	hardPatterns.resize(3);

	float speedRed = 1.5f;
	float speedBlue = 1.25f;
	float speedPupple = 1.0f;

	makeSymmetryBullet(easyPatterns[0], int(GameObjectType::PingPongGameBulletRed), 0.0f, 0.0f, 25, speedRed);
	makeSymmetryBullet(easyPatterns[1], int(GameObjectType::PingPongGameBulletBlue), 0.0f, 0.0f, 17, speedBlue);
	makeSymmetryBullet(easyPatterns[2], int(GameObjectType::PingPongGameBulletPupple), 0.0f, 0.0f, 10, speedPupple);
	makeSymmetryBullet(easyPatterns[3], int(GameObjectType::PingPongGameBulletRed), 0.0f, 0.0f, -10, speedRed);
	makeSymmetryBullet(easyPatterns[4], int(GameObjectType::PingPongGameBulletBlue), 0.0f, 0.0f, -17, speedBlue);
	makeSymmetryBullet(easyPatterns[5], int(GameObjectType::PingPongGameBulletPupple), 0.0f, 0.0f, -25, speedPupple);

	makeSymmetryBullet(mediumPatterns[0], int(GameObjectType::PingPongGameBulletRed), 0.5f, 0.5f, 24, speedRed);
	makeSymmetryBullet(mediumPatterns[0], int(GameObjectType::PingPongGameBulletBlue), -0.5f, -0.5f, -16, speedBlue);
	makeSymmetryBullet(mediumPatterns[1], int(GameObjectType::PingPongGameBulletRed), 0.5f, -0.5f, -24, speedRed);
	makeSymmetryBullet(mediumPatterns[1], int(GameObjectType::PingPongGameBulletPupple), -0.5f, 0.5f, 16, speedPupple);
	makeSymmetryBullet(mediumPatterns[2], int(GameObjectType::PingPongGameBulletBlue), 0.5f, 0.5f, -10, speedBlue);
	makeSymmetryBullet(mediumPatterns[2], int(GameObjectType::PingPongGameBulletPupple), -0.5f, -0.5f, 23, speedPupple);
	makeSymmetryBullet(mediumPatterns[3], int(GameObjectType::PingPongGameBulletRed), -0.5f, -0.5f, -23, speedRed);
	makeSymmetryBullet(mediumPatterns[3], int(GameObjectType::PingPongGameBulletBlue), 0.5f, 0.5f, -10, speedBlue);
	makeSymmetryBullet(mediumPatterns[4], int(GameObjectType::PingPongGameBulletRed), -0.5f, 0.5f, 10, speedRed);
	makeSymmetryBullet(mediumPatterns[4], int(GameObjectType::PingPongGameBulletPupple), 0.5f, -0.5f, -16, speedPupple);
	makeSymmetryBullet(mediumPatterns[5], int(GameObjectType::PingPongGameBulletBlue), -0.5f, -0.5f, -16, speedBlue);
	makeSymmetryBullet(mediumPatterns[5], int(GameObjectType::PingPongGameBulletPupple), 0.5f, 0.5f, 10, speedPupple);

	makeSymmetryBullet(hardPatterns[0], int(GameObjectType::PingPongGameBulletRed), 0.0f, 0.0f, 15, speedRed);
	makeSymmetryBullet(hardPatterns[0], int(GameObjectType::PingPongGameBulletBlue), 0.1f, 0.1f, -15, speedBlue);
	makeSymmetryBullet(hardPatterns[0], int(GameObjectType::PingPongGameBulletPupple), -0.1f, -0.1f, 20, speedPupple);
	makeSymmetryBullet(hardPatterns[1], int(GameObjectType::PingPongGameBulletRed), -0.1f, 0.1f, -15, speedRed);
	makeSymmetryBullet(hardPatterns[1], int(GameObjectType::PingPongGameBulletBlue), 0.0f, 0.0f, 20, speedBlue);
	makeSymmetryBullet(hardPatterns[1], int(GameObjectType::PingPongGameBulletPupple), 0.1f, -0.1f, 15, speedPupple);
	makeSymmetryBullet(hardPatterns[2], int(GameObjectType::PingPongGameBulletRed), 0.0f, -0.1f, 20, speedRed);
	makeSymmetryBullet(hardPatterns[2], int(GameObjectType::PingPongGameBulletBlue), -0.1f, 0.0f, 15, speedBlue);
	makeSymmetryBullet(hardPatterns[2], int(GameObjectType::PingPongGameBulletPupple), 0.0f, 0.0f, -15, speedPupple);
}

void PingPongManager::makeSymmetryBullet(S2C_Protocol::S_P_Bullets& pkt, int32_t bulletType, float px, float pz, int degree, float speed) {
	for (int i = 0; i < 4; i++) {
		S2C_Protocol::S_P_Bullet* pBullet = pkt.add_bullets();
		makeBullet(bulletType, px, pz, GetCos(degree + i * 90), GetSin(degree + i * 90), speed, pBullet);;
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



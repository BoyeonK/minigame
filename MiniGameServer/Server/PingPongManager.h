#pragma once
#include "GameManager.h"
#include "PingPongGameRoom.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PingPongManager : public GameManager {
public:
	PingPongManager() : _ty(GameType::PingPong), _quota(2), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);

		vector<shared_ptr<PingPongGameRoom>> _dummyRooms;
		vector<shared_ptr<PingPongGameBullet>> _dummyBullets;
		for (int i = 0; i < 100; i++) {
			shared_ptr<PingPongGameRoom> room = { objectPool<PingPongGameRoom>::alloc(), objectPool<PingPongGameRoom>::dealloc };
			_dummyRooms.push_back(room);
		}
		for (int i = 0; i < 3333; i++) {
			shared_ptr<PingPongGameBulletRed> bullet = { objectPool<PingPongGameBulletRed>::alloc(), objectPool<PingPongGameBulletRed>::dealloc };
			_dummyBullets.push_back(bullet);
		}
		for (int i = 0; i < 3333; i++) {
			shared_ptr<PingPongGameBulletBlue> bullet = { objectPool<PingPongGameBulletBlue>::alloc(), objectPool<PingPongGameBulletBlue>::dealloc };
			_dummyBullets.push_back(bullet);
		}
		for (int i = 0; i < 3333; i++) {
			shared_ptr<PingPongGameBulletPupple> bullet = { objectPool<PingPongGameBulletPupple>::alloc(), objectPool<PingPongGameBulletPupple>::dealloc };
			_dummyBullets.push_back(bullet);
		}
		_dummyRooms.clear();
		_dummyBullets.clear();
	}

	void Push(WatingPlayerData pd) override;
	void Push(vector<WatingPlayerData> pdv) override;
	void RenewMatchQueue();
	void MatchMake() override;
	void MakeRoom(vector<WatingPlayerData>&& pdv) override;
	bool RenewPublicRecordFromDB() override;
	bool CompareAndRenewPublicRecord(int32_t dbid, int32_t score) override;
	bool TrySetPublicRecord() override { return true; };
	int32_t GetQuota() override { return _quota; }

	void Update() override;

	void StartGame() {}

	void makeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed, S2C_Protocol::S_P_Bullet* outPkt);
	static float GetCos(int degree) {
		double radians = degree * M_PI / 180.0;
		return static_cast<float>(cos(radians));
	}
	static float GetSin(int degree) {
		double radians = degree * M_PI / 180.0;
		return static_cast<float>(sin(radians));
	}
	float GetToleranceRate();

private:
	GameType _ty = GameType::PingPong;
	int32_t _quota = 2;
	MatchQueue _matchQueue;
	vector<bool> _excluded;
	uint64_t _lastRenewMatchQueueTick = 0;
	uint64_t _updateTickPeriod = 100;
	const float _toleranceRate = 0.3;
	uint64_t _pendingRoomsVectorAddTickPeriod = 1000;
	uint64_t _lastPendingRoomsVectorAddTick = 0;
};

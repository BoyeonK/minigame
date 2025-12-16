#pragma once
#include "GameManager.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PingPongManager : public GameManager {
public:
	PingPongManager() : _ty(GameType::PingPong), _quota(4), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
		InitPattern();
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

	void InitPattern();
	void makeSymmetryBullet(S2C_Protocol::S_P_Bullets& pkt, int32_t bulletType, float px, float pz, int degree, float speed);
	void makeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed, S2C_Protocol::S_P_Bullet* outPkt);
	static float GetCos(int degree) {
		double radians = degree * M_PI / 180.0;
		return static_cast<float>(cos(radians));
	}
	static float GetSin(int degree) {
		double radians = degree * M_PI / 180.0;
		return static_cast<float>(sin(radians));
	}
	//4 방향 생성 패턴
	vector<S2C_Protocol::S_P_Bullets> easyPatterns;
	//8 방향 생성 패턴
	vector<S2C_Protocol::S_P_Bullets> mediumPatterns;
	//12+ 방향 패턴
	vector<S2C_Protocol::S_P_Bullets> hardPatterns;
	float GetToleranceRate();

private:
	GameType _ty = GameType::PingPong;
	int32_t _quota = 4;
	MatchQueue _matchQueue;
	vector<bool> _excluded;
	uint64_t _lastRenewMatchQueueTick = 0;
	uint64_t _updateTickPeriod = 100;
	float _toleranceRate = 0.3;
};

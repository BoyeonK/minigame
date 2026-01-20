#pragma once
#include "GameManager.h"

class MoleManager : public GameManager {
public:
	MoleManager() : _ty(GameType::Mole), _quota(1), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
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

private:
	GameType _ty = GameType::Mole;
	int32_t _quota;
	MatchQueue _matchQueue;
	vector<bool> _excluded;
	uint64_t _lastRenewMatchQueueTick = 0;
	uint64_t _updateTickPeriod = 500;
	uint64_t _pendingRoomsVectorAddTickPeriod = 1000;
	uint64_t _lastPendingRoomsVectorAddTick = 0;
};


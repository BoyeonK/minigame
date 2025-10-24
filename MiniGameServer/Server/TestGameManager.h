#pragma once
#include "GameManager.h"

class TestGameManager : public GameManager {
public:
	TestGameManager() : _ty(GameType::TestGame), _quota(1), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
	}

	void Push(WatingPlayerData pd) override;
	void Push(vector<WatingPlayerData> pdv) override;
	void RenewMatchQueue();
	void MatchMake() override;
	void MakeRoom(vector<WatingPlayerData>&& pdv) override;
	bool TrySetPublicRecord(int32_t dbid, int32_t score) override;
	bool TrySetPublicRecordFromDB() override;

	void Update() override;

	void StartGame() {}

private:
	GameType _ty = GameType::TestGame;
	int32_t _quota;
	MatchQueue _matchQueue;
	vector<bool> _excluded;
	uint64_t _lastRenewMatchQueueTick = 0;
	uint64_t _updateTickPeriod = 66;
};

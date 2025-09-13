#pragma once
#include "GameManager.h"

class PingPongManager : public GameManager {
public:
	PingPongManager() : _ty(GameType::PingPong), _quota(4), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
	}

	void Push(WatingPlayerData pd) override;
	void Push(vector<WatingPlayerData> pdv) override;
	void RenewMatchQueue();
	void MatchMake() override;
	void MakeRoom(vector<WatingPlayerData>&& pdv) override;

	void Update() override {}

	void StartGame() {}

private:
	GameType _ty = GameType::PingPong;
	MatchQueue _matchQueue;
	int32_t _quota = 4;
	vector<bool> _excluded;
	uint64_t _lastRenewTick = 0;
};

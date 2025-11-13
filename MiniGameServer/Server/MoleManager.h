#pragma once
#include "GameManager.h"

class MoleManager : public GameManager {
public:
	MoleManager() : _ty(GameType::Mole), _quota(2), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
	}

private:
	GameType _ty = GameType::Mole;
	int32_t _quota = 2;
	MatchQueue _matchQueue;
	vector<bool> _excluded;
	uint64_t _lastRenewMatchQueueTick = 0;
	uint64_t _updateTickPeriod = 100;
};


#pragma once
#include "PlayerSession.h"

struct WatingPlayerData {
	int32_t _elo;
	weak_ptr<PlayerSession> _playerSessionRef;
	uint64_t queuedTick = ::GetTickCount64();

	bool IsValidPlayer() const {
		shared_ptr<PlayerSession> playerSessionRef = _playerSessionRef.lock();
		if (playerSessionRef == nullptr || playerSessionRef->GetMatchingState() != GameType::None)
			return false;
		return true;
	}
};

struct Deviset {
	Deviset(double devi, int32_t idx) : _devi(devi), _idx(idx) {}

	double _devi;
	int32_t _idx;

	bool operator < (const Deviset& other) const {
		return this->_devi > other._devi;
	}
};

class MatchQueue {
public:
	void Push(WatingPlayerData newPlayer);
	void FlushTempQueueAndSort();
	void RemoveInvalidPlayer();
	//void SearchMatchGroup();

	vector<WatingPlayerData> searchQueue;
	vector<bool> selectedChecks;
	vector<int32_t> selectedPlayerIdxs;
	priority_queue<Deviset> pq;
	int32_t allowDevi = 50;
	
private:
	mutex _TQlock;
	vector<WatingPlayerData> _tempQueue;
	mutex _SQlock;
	int32_t _gameNum = 0;
};


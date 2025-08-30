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

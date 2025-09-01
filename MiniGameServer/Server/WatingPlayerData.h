#pragma once
#include "PlayerSession.h"

struct WatingPlayerData {
	int32_t elo;
	weak_ptr<PlayerSession> playerSessionWRef;
	uint64_t queuedTick = ::GetTickCount64();

	bool IsValidPlayer(GameType gameType) const {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		if (playerSessionRef == nullptr || playerSessionRef->GetMatchingState() != gameType)
			return false;
		return true;
	}
};

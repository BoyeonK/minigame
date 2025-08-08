#pragma once
#include "PlayerSession.h"

shared_ptr<PlayerSession> PSfactory(); 

class S2CServerServiceImpl : public ServerService {
public:
	S2CServerServiceImpl(
		shared_ptr<CPCore>CPCoreRef,
		NetAddress address,
		uint32_t maxSessionCount
	) : ServerService (
		CPCoreRef,
		address,
		PSfactory,
		maxSessionCount) {
	}
};


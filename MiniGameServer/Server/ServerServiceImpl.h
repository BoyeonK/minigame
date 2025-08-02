#pragma once
#include "PlayerSession.h"

shared_ptr<PlayerSession> PSfactory(); 

class ServerServiceImpl : public ServerService {
public:
	ServerServiceImpl(
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


#include "pch.h"
#include "S2CServerServiceImpl.h"

shared_ptr<PlayerSession> PSfactory() {
	return make_shared<PlayerSession>();
}

#include "pch.h"
#include "ServerServiceImpl.h"

shared_ptr<PlayerSession> PSfactory() {
	return make_shared<PlayerSession>();
}

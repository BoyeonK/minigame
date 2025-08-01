#include "pch.h"
#include "ServerPacketHandler.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"

shared_ptr<PlayerSession> PSfactory() {
	return make_shared<PlayerSession>();
}

int main() {
	cout << "I'm Server" << endl;

	ServerPacketHandler::Init();

	shared_ptr<ServerService> serverService = make_shared<ServerService>(
		make_shared<CPCore>(),
		NetAddress(L"0.0.0.0", 7777),
		PSfactory,
		100
	);

	serverService->StartAccept();

	GThreadManager->Launch([=]() {
		while (true) {
			ThreadManager::DoTimerQueueDistribution();
			this_thread::sleep_for(20ms);
		}
	});

	for (int i = 0; i < 4; i++) {
		GThreadManager->Launch([=]() {
			while (true) {
				LEndTickCount = ::GetTickCount64() + 64;
				ThreadManager::DoGlobalQueueWork();
				serverService->GetCPCoreRef()->Dispatch(10);
			}
		});
	}

	GThreadManager->Join();
}
#include "pch.h"
#include "ServerPacketHandler.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"
#include "S2D_Protocol.grpc.pb.h"
#include "gRPC_test.h"

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

	string target_address("localhost:50051");
	GreeterClient client(grpc::CreateChannel(target_address, grpc::InsecureChannelCredentials()));

	cout << "Sending async calls..." << endl;

	client.SayHelloAsync("World");
	client.SayHelloAsync("gRPC");
	client.SayHelloAsync("User");

	// 비동기 호출이 완료될 시간을 주기 위해 main 스레드를 잠시 대기
	this_thread::sleep_for(chrono::seconds(2));

	cout << "Client finished." << endl;

	GThreadManager->Join();
}
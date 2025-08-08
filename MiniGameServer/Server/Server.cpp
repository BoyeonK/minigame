#include "pch.h"
#include "S2CPacketHandler.h"
#include "ServerGlobal.h"

int main() {
	//Game Client와의 프로토콜을 정의한 PacketHandler 초기화.
	S2CPacketHandler::Init();

	//DB서버와의 연결 진행
	DBManager = new DBClientImpl(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
#ifdef _DEBUG
	DBManager->HelloAsync();
#endif

	//Client와의 연결을 담당할 서비스 객체 생성 및 Listen시작.
	GServerService = make_shared<S2CServerServiceImpl>(make_shared<CPCore>(), NetAddress(L"0.0.0.0", 7777), 100);
	GServerService->StartAccept();
	
	//Worker Thread 생성
	GThreadManager->Launch([=]() {
		while (true) {
			ThreadManager::DoTimerQueueDistribution();
			this_thread::sleep_for(20ms);
		}
	});

	for (int i = 0; i < 5; i++) {
		GThreadManager->Launch([=]() {
			while (true) {
				LEndTickCount = ::GetTickCount64() + 64;
				ThreadManager::DoGlobalQueueWork();
				DBManager->AsyncCompleteRpc();
				GServerService->GetCPCoreRef()->Dispatch(10);
			}
		});
	}

	GThreadManager->Join();
}
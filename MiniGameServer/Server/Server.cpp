#include "pch.h"
#include "S2CPacketHandler.h"
#include "ServerGlobal.h"
#include <grpcpp/grpcpp.h>

int main() {
	//Game Client와의 프로토콜을 정의한 PacketHandler 초기화
	S2CPacketHandler::Init();

	//DB랑 gRPC통신 SSL인증서 로드
	string certPath = GEnvManager->GetEnv("CERTNAME");
	string root_cert = GEnvManager->ReadFile(certPath);

	grpc::SslCredentialsOptions ssl_opts;
	ssl_opts.pem_root_certs = root_cert;

	grpc::ChannelArguments args;
	string targetName = GEnvManager->GetEnv("CNNAME");
	args.SetSslTargetNameOverride(targetName);

	auto channel = grpc::CreateCustomChannel(
		GEnvManager->GetEnv("DB_ADDRESS"),
		grpc::SslCredentials(ssl_opts),
		args
	);

	//DB gRPC통신 Channel생성 및 최초 정상 작동 확인하기
	DBManager = new DBClientImpl(channel);
	DBManager->HelloAsync();

	//Client와의 연결을 담당할 서비스 객체 생성 및 Listen시작.
	GServerService = make_shared<S2CServerServiceImpl>(make_shared<CPCore>(), NetAddress(L"0.0.0.0", 7777), 100);
	GServerService->StartAccept();
	
	//Timer Queue만을 담당할 worker thread
	GThreadManager->Launch([=]() {
		while (true) {
			ThreadManager::DoTimerQueueDistribution();
			this_thread::sleep_for(20ms);
		}
	});

	//최고 기록 불러오기 (본디 Redis로 만들어야 했을 부분)
	for (auto& gameManager : GGameManagers) {
		gameManager.second->RenewPublicRecordFromDB();
	}

	//Worker Thread 생성
	//AWS t3.micro 기준 thread 2개
	for (int i = 0; i < 2; i++) {
		GThreadManager->Launch([=]() {
			while (true) {
				LEndTickCount = ::GetTickCount64() + 64;
				ThreadManager::DoGlobalQueueWork();
				DBManager->AsyncCompleteRpc();
				GServerService->GetCPCoreRef()->Dispatch(10);
			}
		});
	}
	
	//매치메이킹 및 게임룸 생명주기 관리 담당 Thread
	GThreadManager->Launch([=]() {
		while (true) {
			for (int i = 1; i <= 3; i++) {
				GGameManagers[i]->MatchMake();
				GGameManagers[i]->RenewMatchQueue();
				GGameManagers[i]->RemoveInvalidRoom();
				GGameManagers[i]->Update();
			}
			this_thread::sleep_for(20ms);
		}
	});

	std::cout << "Server is running..." << endl;

	GThreadManager->Join();
}
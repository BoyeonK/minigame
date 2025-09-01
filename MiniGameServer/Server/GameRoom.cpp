#include "pch.h"
#include "GameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void TestMatchGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd._playerSessionRef.lock();
		_playerWRefs.push_back(pd._playerSessionRef);
		if (playerSessionRef == nullptr) {
			ready = false;
			break;
		}
		//각 Session에 KeepAlive패킷을 BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(1);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
	}

	if (ready) {
		//1초 후, (Ping이 1초가 넘는것은, 이상하다.) 모든 패킷으로부터 응답을 받았다면 시작
		DoTimerAsync(1000, &TestMatchGameRoom::Init2, move(pdv));
	}
	else {
		//유효하지 않은 세션이 있었을 경우, 모두 대기열로 돌려보냄.
		//대기열은 주기적으로 유효하지 않은 PlayerData를 거르도록 설계되어있음.
		GGameManagers[1]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void TestMatchGameRoom::Init2(vector<WatingPlayerData> pdv) {
	bool canStart = true;
	for (auto& playerSessionWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		//유효하지 않은 플레이어가 있는 경우, 모두 대기열로 돌려보냄.
		if (playerSessionRef == nullptr) {
			canStart = false;
			break;
		}
		int64_t now = ::GetTickCount64();
		int64_t lastTick = playerSessionRef->GetLastKeepAliveTick();

		//C_KeepAlive Handler함수에 의해서 lastTick이 변화하지 않았다면,
		//유효하지 않은 플레이어로 간주하고, 모두 대기열로 돌려보냄.
		if (now - lastTick > 1000) {
			canStart = false;
		}
	}

	if (canStart) {
		//이제는 정말 게임을 진행할 것임.
		//지금부터는 해당 플레이어의 게임종료 등의 이유로 세션이 유효하지 않더라도
		//진행 가능한 방식으로 코드를 작성해야 함.
		_state = GameState::BeforeStart;

		//TODO : S_MatchMakeComplete 패킷을 broadcast해서 Scene변경을 유도 및, 로딩 진행 정도에 따라 C_GameSceneLoadingProgress패킷을 전송받음.
		//모든 유저의 Loading이 완료되거나, 일정 시간이 지난 경우 게임 시작.
		Start();
	}
	else {
		GGameManagers[1]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void TestMatchGameRoom::ReturnToPool() {
	objectPool<TestMatchGameRoom>::dealloc(this);
}

void PingPongGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	vector<WatingPlayerData> rematchPd;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd._playerSessionRef.lock();

		if (playerSessionRef == nullptr) {
			ready = false;
			continue;
		}
		//각 Session에 KeepAlive패킷을 BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(2);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
		rematchPd.push_back(pd);
	}

	if (ready) {
		//1초 후, (Ping이 1초가 넘는것은, 이상하다.) 모든 패킷으로부터 응답을 받았다면 시작
		DoTimerAsync(1000, &PingPongGameRoom::Init2, move(pdv));
	}
	else {
		GGameManagers[2]->Push(rematchPd);
	}
}

void PingPongGameRoom::Init2(vector<WatingPlayerData> pdv) {

}

void PingPongGameRoom::ReturnToPool() {
	objectPool<PingPongGameRoom>::dealloc(this);
}

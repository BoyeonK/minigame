#include "pch.h"
#include "TestGameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "TestGameBullet.h"

void TestGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	cout << "룸 생성, Init1" << endl;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();
		_playerWRefs.push_back(pd.playerSessionWRef);
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
		PostEventAfter(1000, &TestGameRoom::Init2, move(pdv));
	}
	else {
		//유효하지 않은 세션이 있었을 경우, 모두 대기열로 돌려보냄.
		//대기열은 주기적으로 유효하지 않은 PlayerData를 거르도록 설계되어있음.
		GGameManagers[1]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

void TestGameRoom::Init2(vector<WatingPlayerData> pdv) {
	cout << "Init2" << endl;

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
		if (now - lastTick > 2000) {
			canStart = false;
			break;
		}
	}

	if (canStart) {
		//이제는 정말 게임을 진행할 것임.
		//지금부터 연결상태가 좋지 않으면 플레이어 책임으로 간주.
		//플레이어의 게임종료 등의 이유로 세션이 유효하지 않더라도, 진행 가능한 방식으로 코드를 작성해야 함.
		_state = GameState::BeforeStart;
		_preparedPlayer = 0;
		for (auto& playerSessionWRef : _playerWRefs) {
			shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
			S2C_Protocol::S_MatchmakeCompleted pkt = S2CPacketMaker::MakeSMatchmakeCompleted(int(_ty));
			if (playerSessionRef != nullptr) {
				playerSessionRef->SetJoinedRoom(static_pointer_cast<TestGameRoom>(shared_from_this()));
				shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
				playerSessionRef->Send(sendBuffer);
			}
		}
	}
	else {
		cout << "영 좋지 않음" << endl;
		GGameManagers[1]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void TestGameRoom::Start() {
	_state = GameState::OnGoing;
	cout << "스타트 함수 실행" << endl;
	//TODO : 완료됨을 전파
	for (auto& playerSessionWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		S2C_Protocol::S_GameStarted pkt = S2CPacketMaker::MakeSGameStarted(int(_ty));
		if (playerSessionRef != nullptr) {
			shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
			playerSessionRef->Send(sendBuffer);
		}
	}
	PostEventAfter(3000, &TestGameRoom::Phase1);
}

void TestGameRoom::SendGameState(int32_t playerIdx) {
	if (playerIdx > (_quota - 1))
		return;

	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (playerSessionRef == nullptr)
		return;

	S2C_Protocol::S_TestGameState pkt = MakeSTestGameState();
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	playerSessionRef->Send(sendBuffer);
}

void TestGameRoom::MakeTestGameBullet(float x, float y, float z) {
	shared_ptr<TestGameBullet> bulletRef = TestGameBullet::NewTestGameBullet(x, y, z);
	bulletRef->SetObjectId(GenerateUniqueGameObjectId());
	RegisterGameObject(bulletRef);
	S2C_Protocol::S_SpawnGameObject pkt = S2CPacketMaker::MakeSSpawnGameObject(bulletRef);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void TestGameRoom::Phase1() {
	DispatchEvent(&TestGameRoom::MakeTestGameBullet, -2.0f, 0.0f, 0.0f);
	PostEventAfter(1000, &TestGameRoom::MakeTestGameBullet, -1.0f, 0.0f, 0.0f);
	PostEventAfter(2000, &TestGameRoom::MakeTestGameBullet, 0.0f, 0.0f, 0.0f);
	PostEventAfter(3000, &TestGameRoom::MakeTestGameBullet, 1.0f, 0.0f, 0.0f);
	PostEventAfter(4000, &TestGameRoom::MakeTestGameBullet, 2.0f, 0.0f, 0.0f);
	PostEventAfter(6000, &TestGameRoom::EndPhase);
}

void TestGameRoom::EndPhase() {
	//TestGame에서는 아무 결과도 계산하지 않고, 게임이 끝났다는 정보 외에는 아무른 결과도 알려주지 않음.

	//게임 결과 계산
	CalculateGameResult();

	//해당 결과 통보
	S2C_Protocol::S_TestGameEnd pkt = S2CPacketMaker::MakeSTestGameEnd();
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void TestGameRoom::CalculateGameResult() {
	
}

S2C_Protocol::S_TestGameState TestGameRoom::MakeSTestGameState() {
	S2C_Protocol::S_TestGameState pkt;
	for (auto& goRef : _vecGameObjects) {
		goRef->SerializeObject(pkt.add_objects());
	}
	return pkt;
}

void TestGameRoom::ReturnToPool() {
	objectPool<TestGameRoom>::dealloc(this);
}

void TestGameRoom::UpdateProgressBar(int32_t playerIdx, int32_t progressRate) {
	cout << "업데이트 프로그레스 바" << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}
	//TODO : 로딩 진행상황 전파

	if (_preparedPlayer == _quota) {
		Start();
	}
}
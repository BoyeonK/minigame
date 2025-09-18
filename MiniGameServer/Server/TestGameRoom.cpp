#include "pch.h"
#include "TestGameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "TestGameBullet.h"

void TestGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	cout << "�� ����, Init1" << endl;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();
		_playerWRefs.push_back(pd.playerSessionWRef);
		if (playerSessionRef == nullptr) {
			ready = false;
			break;
		}
		//�� Session�� KeepAlive��Ŷ�� BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(1);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
	}

	if (ready) {
		//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
		PostEventAfter(1000, &TestGameRoom::Init2, move(pdv));
	}
	else {
		//��ȿ���� ���� ������ �־��� ���, ��� ��⿭�� ��������.
		//��⿭�� �ֱ������� ��ȿ���� ���� PlayerData�� �Ÿ����� ����Ǿ�����.
		GGameManagers[1]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

void TestGameRoom::Init2(vector<WatingPlayerData> pdv) {
	cout << "Init2" << endl;

	bool canStart = true;
	for (auto& playerSessionWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		//��ȿ���� ���� �÷��̾ �ִ� ���, ��� ��⿭�� ��������.
		if (playerSessionRef == nullptr) {
			canStart = false;
			break;
		}
		int64_t now = ::GetTickCount64();
		int64_t lastTick = playerSessionRef->GetLastKeepAliveTick();

		//C_KeepAlive Handler�Լ��� ���ؼ� lastTick�� ��ȭ���� �ʾҴٸ�,
		//��ȿ���� ���� �÷��̾�� �����ϰ�, ��� ��⿭�� ��������.
		if (now - lastTick > 2000) {
			canStart = false;
			break;
		}
	}

	if (canStart) {
		//������ ���� ������ ������ ����.
		//���ݺ��� ������°� ���� ������ �÷��̾� å������ ����.
		//�÷��̾��� �������� ���� ������ ������ ��ȿ���� �ʴ���, ���� ������ ������� �ڵ带 �ۼ��ؾ� ��.
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
		cout << "�� ���� ����" << endl;
		GGameManagers[1]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void TestGameRoom::Start() {
	_state = GameState::OnGoing;
	cout << "��ŸƮ �Լ� ����" << endl;
	//TODO : �Ϸ���� ����
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
	//TestGame������ �ƹ� ����� ������� �ʰ�, ������ �����ٴ� ���� �ܿ��� �ƹ��� ����� �˷����� ����.

	//���� ��� ���
	CalculateGameResult();

	//�ش� ��� �뺸
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
	cout << "������Ʈ ���α׷��� ��" << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}
	//TODO : �ε� �����Ȳ ����

	if (_preparedPlayer == _quota) {
		Start();
	}
}
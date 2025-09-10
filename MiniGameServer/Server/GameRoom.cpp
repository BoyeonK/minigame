#include "pch.h"
#include "GameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void TestMatchGameRoom::Init(vector<WatingPlayerData> pdv) {
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
		DoTimerAsync(1000, &TestMatchGameRoom::Init2, move(pdv));
	}
	else {
		//��ȿ���� ���� ������ �־��� ���, ��� ��⿭�� ��������.
		//��⿭�� �ֱ������� ��ȿ���� ���� PlayerData�� �Ÿ����� ����Ǿ�����.
		GGameManagers[1]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

GameRoom::GameState GameRoom::GetState() {
	return _state;
}

void TestMatchGameRoom::Init2(vector<WatingPlayerData> pdv) {
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
				playerSessionRef->SetJoinedRoom(static_pointer_cast<TestMatchGameRoom>(shared_from_this()));
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

void TestMatchGameRoom::Start() {
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
}

void TestMatchGameRoom::ReturnToPool() {
	objectPool<TestMatchGameRoom>::dealloc(this);
}

void TestMatchGameRoom::UpdateProgressBar(int32_t playerIdx, int32_t progressRate) {
	cout << "������Ʈ ���α׷��� ��" << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}
	//TODO : �ε� �����Ȳ ����

	if (_preparedPlayer == _quota) {
		Start();
	}
}

void PingPongGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	vector<WatingPlayerData> rematchPd;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();

		if (playerSessionRef == nullptr) {
			ready = false;
			continue;
		}
		//�� Session�� KeepAlive��Ŷ�� BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(2);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
		rematchPd.push_back(pd);
	}

	if (ready) {
		//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
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

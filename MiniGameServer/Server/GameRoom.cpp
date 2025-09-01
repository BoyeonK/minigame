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
		GGameManagers[1]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void TestMatchGameRoom::Init2(vector<WatingPlayerData> pdv) {
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
		if (now - lastTick > 1000) {
			canStart = false;
		}
	}

	if (canStart) {
		//������ ���� ������ ������ ����.
		//���ݺ��ʹ� �ش� �÷��̾��� �������� ���� ������ ������ ��ȿ���� �ʴ���
		//���� ������ ������� �ڵ带 �ۼ��ؾ� ��.
		_state = GameState::BeforeStart;

		//TODO : S_MatchMakeComplete ��Ŷ�� broadcast�ؼ� Scene������ ���� ��, �ε� ���� ������ ���� C_GameSceneLoadingProgress��Ŷ�� ���۹���.
		//��� ������ Loading�� �Ϸ�ǰų�, ���� �ð��� ���� ��� ���� ����.
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

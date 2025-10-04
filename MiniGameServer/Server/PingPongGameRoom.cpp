#include "pch.h"
#include "PingPongGameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void PingPongGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	cout << "PingPong �� ����" << endl;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();
		_playerWRefs.push_back(pd.playerSessionWRef);
		if (playerSessionRef == nullptr) {
			ready = false;
			break;
		}
		//�� Session�� KeepAlive��Ŷ�� BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(int(_ty));
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
	}

	if (ready) {
		//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
		PostEventAfter(1000, &PingPongGameRoom::Init2, move(pdv));
	}
	else {
		//��ȿ���� ���� ������ �־��� ���, ��� ��⿭�� ��������.
		//��⿭�� �ֱ������� ��ȿ���� ���� PlayerData�� �Ÿ����� ����Ǿ�����.
		GGameManagers[int(_ty)]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

void PingPongGameRoom::Init2(vector<WatingPlayerData> pdv) {
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
		int playerIdx = 0;
		for (auto& playerSessionWRef : _playerWRefs) {
			shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
			S2C_Protocol::S_MatchmakeCompleted pkt = S2CPacketMaker::MakeSMatchmakeCompleted(int(_ty));
			if (playerSessionRef != nullptr) {
				playerSessionRef->SetJoinedRoom(static_pointer_cast<PingPongGameRoom>(shared_from_this()));
				playerSessionRef->SetRoomIdx(playerIdx++);
				shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
				playerSessionRef->Send(sendBuffer);
			}
		}
		//30�� �ڿ��� ������ �����ع���
		PostEventAfter(30000, &PingPongGameRoom::Start);
	}
	else {
		cout << "���� ���� �Ұ���." << endl;
		GGameManagers[int(_ty)]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void PingPongGameRoom::UpdateProgressBar(int32_t playerIdx, int32_t progressRate) {
	cout << "������Ʈ ���α׷��� ��" << endl;
	cout << "_quota : " << _quota << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}
	cout << "_preparedPlayer : " << _preparedPlayer << endl;
	//TODO : �ε� �����Ȳ ����

	if (_preparedPlayer == _quota) {
		Start();
	}
}

void PingPongGameRoom::Start() {
	if (_state != GameState::BeforeStart)
		return;
	_state = GameState::OnGoing;

	cout << "��ŸƮ �Լ� ����" << endl;
	S2C_Protocol::S_GameStarted pkt = S2CPacketMaker::MakeSGameStarted(int(_ty));
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void PingPongGameRoom::SendGameState(int32_t playerIdx) {
	if (playerIdx > (_quota - 1))
		return;

	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (playerSessionRef == nullptr)
		return;

	//0:�� 1:�� 2:�� 3:��
	S2C_Protocol::S_P_State pkt;
	pkt.set_playerid(playerIdx);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	playerSessionRef->Send(sendBuffer);
}

void PingPongGameRoom::ReturnToPool() {
	objectPool<PingPongGameRoom>::dealloc(this);
}

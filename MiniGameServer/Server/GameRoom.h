#pragma once
#include "MatchQueue.h"

class PlayerSession;

class GameRoom : public JobQueue {
public:
	virtual void Init(vector<WatingPlayerData> pdv) = 0;
	virtual void Update() = 0;

	vector<weak_ptr<PlayerSession>> _playerWRefs;
};

class PingPongGameRoom : public GameRoom {
	//BeforeInit : Room�� ���� ������ ���.
	//BeforeStart : Player�� �ε� �� KeepAlive���� ��Ȯ��
	//OnGoing : ������ �������� ���. (���⼭ �� ����ȭ �� ���� ����)
	//Counting : ���� ����. ����� ���� ���������� DB�� �ݿ��ϰ� Room ����.
	enum class GameState {
		BeforeInit,
		BeforeStart,
		OnGoing,
		Counting,
	};

public:
	void Init(vector<WatingPlayerData> pdv) override {
		//�� Session�� KeepAlive��Ŷ�� BroadCast.
		//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
		//��� Session���κ��� ������ ���� ���ߴٸ� 
			//������ Session�� �ٽ� ��⿭��...
			//�������� ���� Session�� ��ġ�� ��� or ������ ����.
		bool ready = true;
		vector<WatingPlayerData> rematchPd;
		
		for (auto& pd : pdv) {
			shared_ptr<PlayerSession> playerSessionRef = pd._playerSessionRef.lock();

			if (playerSessionRef == nullptr) {
				ready = false;
				continue;
			}
			/*
			S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(1);
			shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
			playerSessionRef->Send(sendBuffer);
			rematchPd.push_back(pd);
			*/
		}

		if (ready) {

		}
	}

	void Update() override {
		
	}

private:
	int32_t _quota = 4;
	GameType _ty = GameType::PingPong;
	GameState _state = GameState::BeforeInit;
};
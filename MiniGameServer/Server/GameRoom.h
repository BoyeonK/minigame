#pragma once
#include "MatchQueue.h"

class PlayerSession;

class GameRoom : public JobQueue {
public:
	virtual void Start(vector<WatingPlayerData> pdv) = 0;
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
	void Start(vector<WatingPlayerData> pdv) override {
		//�� Session�� KeepAlive��Ŷ�� BroadCast.
		//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
		//��� Session���κ��� ������ ���� ���ߴٸ� 
			//������ Session�� �ٽ� ��⿭��...
			//�������� ���� Session�� ��ġ�� ��� or ������ ����.
	}

	void Update() override {
		
	}

private:
	int32_t _quota = 4;
	GameType _ty = GameType::PingPong;
	GameState _state = GameState::BeforeInit;
};
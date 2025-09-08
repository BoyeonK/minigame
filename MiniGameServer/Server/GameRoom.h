#pragma once
#include "MatchQueue.h"

class PlayerSession;

class GameRoom : public JobQueue {
public:
	//BeforeInit : Room�� ���� ������ ���.
	//BeforeStart : Player�� �ε� �� KeepAlive���� ��Ȯ��
	//OnGoing : ������ �������� ���. (���⼭ �� ����ȭ �� ���� ����)
	//Counting : ���� ����. ����� ���� ���������� DB�� �ݿ��ϰ� Room ����.
	enum class GameState {
		BeforeInit,
		BeforeStart,
		OnGoing,
		Counting,
		EndGame,
	};

	virtual void Init(vector<WatingPlayerData> pdv) = 0;
	virtual void Update() = 0;
	GameState GetState();
	
protected:
	vector<weak_ptr<PlayerSession>> _playerWRefs;
	GameType _ty;
	GameState _state = GameState::BeforeInit;
};

class TestMatchGameRoom : public GameRoom {
public:
	TestMatchGameRoom() {
		_ty = GameType::TestMatch;
	}
	~TestMatchGameRoom() {
		cout << "�� ���" << endl;
	}

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);
	void Start();
	void ReturnToPool();
	void Update() override {}

private:
	int32_t _quota = 1;
};

class PingPongGameRoom : public GameRoom {
public:
	PingPongGameRoom() {
		_ty = GameType::PingPong;
	}

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);
	void ReturnToPool();
	void Update() override { }

private:
	int32_t _quota = 4;
};
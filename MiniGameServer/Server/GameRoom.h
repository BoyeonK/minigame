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
	};

	virtual void Init(vector<WatingPlayerData> pdv) = 0;
	virtual void Update() = 0;

	vector<weak_ptr<PlayerSession>> _playerWRefs;
};

class TestMatchGameRoom : public GameRoom {
public:
	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);
	void ReturnToPool();
	void Update() override {}

private:
	int32_t _quota = 1;
	GameType _ty = GameType::TestMatch;
	GameState _state = GameState::BeforeInit;
};

class PingPongGameRoom : public GameRoom {
public:
	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);
	void ReturnToPool();
	void Update() override { }

private:
	int32_t _quota = 4;
	GameType _ty = GameType::PingPong;
	GameState _state = GameState::BeforeInit;
};
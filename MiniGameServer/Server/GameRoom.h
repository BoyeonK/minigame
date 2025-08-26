#pragma once
#include "GameType.h"
#include "GameManager.h"

class PlayerSession;

class GameRoom : public JobQueue {
public:
	virtual void Start(vector<weak_ptr<PlayerSession>> psv) = 0;
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
	void Start(vector<weak_ptr<PlayerSession>> psv) override {

	}

	void Update() override {
		
	}

private:
	int32_t _quota = 4;
	GameType _ty = GameType::PingPong;
	GameState _state = GameState::BeforeInit;
};
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
	//BeforeInit : Room이 최초 생성된 경우.
	//BeforeStart : Player의 로딩 및 KeepAlive여부 재확인
	//OnGoing : 게임이 진행중인 경우. (여기서 더 세분화 될 수도 있음)
	//Counting : 게임 종료. 결과에 따른 변동사항을 DB에 반영하고 Room 종료.
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
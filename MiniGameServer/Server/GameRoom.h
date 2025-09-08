#pragma once
#include "MatchQueue.h"

class PlayerSession;

class GameRoom : public JobQueue {
public:
	//BeforeInit : Room이 최초 생성된 경우.
	//BeforeStart : Player의 로딩 및 KeepAlive여부 재확인
	//OnGoing : 게임이 진행중인 경우. (여기서 더 세분화 될 수도 있음)
	//Counting : 게임 종료. 결과에 따른 변동사항을 DB에 반영하고 Room 종료.
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
		cout << "룸 사망" << endl;
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
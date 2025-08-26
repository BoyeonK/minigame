#pragma once
#include "PlayerSession.h"
#include "GameRoom.h"

class GameManager {
protected:
	void MatchMake(vector<weak_ptr<PlayerSession>> psv) {
		//TODO : psv안에 모든 친구들이 유효한 친구들인지 확인.
		//유효하면 해당 vector로서 MakeRoom을 실행.
	}

	virtual void MakeRoom(vector<weak_ptr<PlayerSession>> psv) = 0;
	virtual void Update() = 0;
	vector<shared_ptr<GameRoom>> _rooms;
};

class PingPongManager : public GameManager {
	enum class GameState {
		BeforeInit,
		BeforeStart,
		OnGoing,
		Counting,
	};

public:
	void MakeRoom(vector<weak_ptr<PlayerSession>> psv) override {

	}

	void Update() override {
		
	}
	
	void StartGame() {

	}
	
private:
	int32_t _quota = 4;
	GameState _state = GameState::BeforeInit;
};

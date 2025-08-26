#pragma once
#include "PlayerSession.h"
#include "GameRoom.h"

class GameManager {
protected:
	void MatchMake(vector<weak_ptr<PlayerSession>> psv) {
		//TODO : psv�ȿ� ��� ģ������ ��ȿ�� ģ�������� Ȯ��.
		//��ȿ�ϸ� �ش� vector�μ� MakeRoom�� ����.
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

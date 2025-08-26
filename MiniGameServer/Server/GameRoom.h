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
	void Start(vector<WatingPlayerData> pdv) override {
		//각 Session에 KeepAlive패킷을 BroadCast.
		//1초 후, (Ping이 1초가 넘는것은, 이상하다.) 모든 패킷으로부터 응답을 받았다면 시작
		//모든 Session으로부터 응답을 받지 못했다면 
			//응답한 Session은 다시 대기열로...
			//응답하지 않은 Session의 매치를 취소 or 연결을 종료.
	}

	void Update() override {
		
	}

private:
	int32_t _quota = 4;
	GameType _ty = GameType::PingPong;
	GameState _state = GameState::BeforeInit;
};
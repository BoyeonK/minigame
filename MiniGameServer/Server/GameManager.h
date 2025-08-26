#pragma once
#include <shared_mutex>
#include "GameRoom.h"
#include "PlayerSession.h"
#include "GameType.h"

class GameManager {
protected:
	//TODO : psv안에 모든 친구들이 유효한 친구들인지 확인.
	//유효하면 해당 vector로서 MakeRoom을 실행.
	virtual void MatchMake(vector<weak_ptr<PlayerSession>> psv) = 0;
	virtual void MakeRoom(vector<weak_ptr<PlayerSession>> psv) = 0;
	virtual void Update() = 0;
	void AddRoom(shared_ptr<GameRoom> room) {
		unique_lock<shared_mutex> lock(_roomsLock);
		_rooms.push_back(room);
	}

	vector<shared_ptr<GameRoom>> _rooms;
	shared_mutex _roomsLock;
};

class PingPongManager : public GameManager {
public:
	void MatchMake(vector<weak_ptr<PlayerSession>> psv) override {
		bool isReady = true;
		for (auto& wref : psv) {
			shared_ptr<PlayerSession> playerSessionRef = wref.lock();
			if (playerSessionRef == nullptr || playerSessionRef->GetMatchingState() != GameType::PingPong) {
				isReady = false;
				break;
			}
		}

		if (isReady) 
			MakeRoom(psv);
	}

	void MakeRoom(vector<weak_ptr<PlayerSession>> psv) override {
		shared_ptr<PingPongGameRoom> newRoom = { objectPool<PingPongGameRoom>::alloc(), objectPool<PingPongGameRoom>::dealloc };
		newRoom->DoAsync(PingPongGameRoom::Start, psv);
		AddRoom(newRoom);
	}

	void Update() override {
		
	}
	
	void StartGame() { }
	
private:
	GameType _ty = GameType::PingPong;
};

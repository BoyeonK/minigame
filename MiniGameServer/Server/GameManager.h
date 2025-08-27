#pragma once
#include <shared_mutex>
#include "MatchQueue.h"
#include "GameRoom.h"
#include "PlayerSession.h"

class GameManager {
protected:
	//TODO : psv�ȿ� ��� ģ������ ��ȿ�� ģ�������� Ȯ��.
	//��ȿ�ϸ� �ش� vector�μ� MakeRoom�� ����.
	virtual void MatchMake() = 0;
	virtual void MakeRoom(vector<WatingPlayerData>&& pdv) = 0;
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
	void MatchMake() override {
		vector<vector<WatingPlayerData>> pdvv = _matchQueue.SearchMatchGroups();
		for (auto& pdv : pdvv) {
			bool isReady = true;
			for (auto& pd : pdv) {
				shared_ptr<PlayerSession> playerSessionRef = pd._playerSessionRef.lock();
				if (playerSessionRef == nullptr || playerSessionRef->GetMatchingState() != GameType::PingPong) {
					isReady = false;
					//TODO : �ش� playerSession�� ���� ��⿭���� ����
						//pdv���� �ش� player data�� ����
						//�ʿ��ϴٸ�, ���ܵ� ����� Client�� �뺸.
				}
			}

			if (isReady) {
				MakeRoom(move(pdv));
			}
			else {
				for (auto& playerData : pdv) {
					_matchQueue.Push(playerData);
				}
			}
		}
	}

	void MakeRoom(vector<WatingPlayerData>&& pdv) override {
		shared_ptr<PingPongGameRoom> newRoom = { objectPool<PingPongGameRoom>::alloc(), objectPool<PingPongGameRoom>::dealloc };
		newRoom->DoAsyncAfter(&PingPongGameRoom::Start, pdv);
	}

	void Update() override {
		
	}
	
	void StartGame() { }
	
private:
	GameType _ty = GameType::PingPong;
	MatchQueue _matchQueue;
    int32_t _quota = 4;
};

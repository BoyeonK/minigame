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
	PingPongManager() {
		_excluded = vector<bool>(_quota);
	}

	void MatchMake() override {
		vector<vector<WatingPlayerData>> pdvv = _matchQueue.SearchMatchGroups();
		for (auto& pdv : pdvv) {
			bool isReady = true;
			fill(_excluded.begin(), _excluded.end(), false);
			for (int i = 0; i < _quota; i++) {
				shared_ptr<PlayerSession> playerSessionRef = pdv[i]._playerSessionRef.lock();
				if (playerSessionRef == nullptr || playerSessionRef->GetMatchingState() != GameType::PingPong) {
					isReady = false;
					_excluded[i] = true;

					//S2DPacketMaker
					//playerSessionRef->Send()
					//TODO : �ش� playerSession�� ���� ��⿭���� ����
						//�ʿ��ϴٸ�, ���ܵ� ����� Client�� �뺸.
				}
			}

			if (isReady) {
				MakeRoom(move(pdv));
			}
			else {
				for (int i = 0; i < _quota; i++) {
					if (!_excluded[i])
						_matchQueue.Push(move(pdv[i]));
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
	vector<bool> _excluded;
};

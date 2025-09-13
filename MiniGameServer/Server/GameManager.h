#pragma once
#include "WatingPlayerData.h"
#include "MatchQueue.h"

class GameManager {
public:
	//TODO : psv�ȿ� ��� ģ������ ��ȿ�� ģ�������� Ȯ��.
	//��ȿ�ϸ� �ش� vector�μ� MakeRoom�� ����.
	virtual void Push(WatingPlayerData pd) = 0;
	virtual void Push(vector<WatingPlayerData> pdv) = 0;
	virtual void RenewMatchQueue() = 0;
	virtual void MatchMake() = 0;
	virtual void MakeRoom(vector<WatingPlayerData>&& pdv) = 0;
	virtual void Update() = 0;

	void AddRoom(shared_ptr<GameRoom> room);
	void RemoveInvalidRoom();

protected:
	uint64_t _lastRenewRoomTick = 0;
	vector<shared_ptr<GameRoom>> _rooms;
	shared_mutex _roomsLock;
};


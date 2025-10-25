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
	int32_t GetPublicRecord();
	void SetPublicRecord(string playerId, int32_t record);
	virtual bool RenewPublicRecordFromDB() = 0;
	virtual bool CompareAndRenewPublicRecord(int32_t dbid, int32_t score) = 0;
	virtual bool TrySetPublicRecord() = 0;

	void AddRoom(shared_ptr<GameRoom> room);
	void RemoveInvalidRoom();

protected:
	mutex _recordLock;
	atomic<int32_t> _nxtRoomId = 0;
	uint64_t _lastRemoveRoomTick = 0;
	uint64_t _lastUpdateRoomTick = 0;
	vector<shared_ptr<GameRoom>> _rooms;
	shared_mutex _roomsLock;
	atomic<int32_t> _publicRecord = 0;
	string _publicRecorder;
};


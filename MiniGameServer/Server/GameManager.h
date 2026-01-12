#pragma once
#include "WatingPlayerData.h"
#include "MatchQueue.h"

class GameManager {
public:
	//TODO : psv안에 모든 친구들이 유효한 친구들인지 확인.
	//유효하면 해당 vector로서 MakeRoom을 실행.
	virtual void Push(WatingPlayerData pd) = 0;
	virtual void Push(vector<WatingPlayerData> pdv) = 0;
	virtual void RenewMatchQueue() = 0;
	virtual void MatchMake() = 0;
	virtual void MakeRoom(vector<WatingPlayerData>&& pdv) = 0;
	virtual void Update() = 0;
	int32_t GetPublicRecord();
	string GetPublicRecorder();
	void SetPublicRecord(string playerId, int32_t record);
	virtual bool RenewPublicRecordFromDB() = 0;
	virtual bool CompareAndRenewPublicRecord(int32_t dbid, int32_t score) = 0;
	virtual bool TrySetPublicRecord() = 0;
	virtual int32_t GetQuota() = 0;

	void AddRoom(shared_ptr<GameRoom> room);
	void AddRoomsFromPendingVector();
	void RemoveInvalidRoom();

protected:
	mutex _recordLock;
	atomic<int32_t> _nxtRoomId = 0;
	uint64_t _lastRemoveRoomTick = 0;
	uint64_t _removeRoomTickPeriod = 5000;
	uint64_t _lastUpdateRoomTick = 0;

	mutex _roomsLock;
	vector<shared_ptr<GameRoom>> _rooms;
	int32_t _roomCount = 0;
	
	mutex _roomsToBeAddedLock;
	vector<shared_ptr<GameRoom>> _roomsToBeAdded;
	
	atomic<int32_t> _publicRecord = 0;
	string _publicRecorder;
};


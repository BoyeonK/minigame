#pragma once
#include <unordered_map>
#include <cstring>
#include "DBClientImpl.h"
#include "S2CServerServiceImpl.h"
#include "GameRoom.h"
#include "WatingPlayerData.h"

extern class CryptoManager* GCryptoManager;
extern class DBClientImpl* DBManager;
extern class shared_ptr<S2CServerServiceImpl> GServerService;

class CryptoManager {
public:
	CryptoManager();
	~CryptoManager();

	EVP_PKEY* PopKey();
	bool ReturnKey(EVP_PKEY*& key);

	static vector<unsigned char> ExtractPublicKey(EVP_PKEY* key);
	static vector<unsigned char> Decrypt(EVP_PKEY* privateKey, const vector<unsigned char>& encrypted);
	static bool Encrypt(
		const vector<unsigned char>& key, 
		const vector<unsigned char>& plaintext, 
		const int32_t pktId, 
		vector<unsigned char>& iv, 
		vector<unsigned char>& ciphertext, 
		vector<unsigned char>& tag);


private:
	USE_RWLOCK;
	queue<EVP_PKEY*> _keyQueue;

	//숫자를 세는 것에 더해서 unordered set을 사용하여
	//사용, 비사용된 key를 정확히 추적할 수도 있음
	//나중에 수요가 생기면 추가하는 걸로 하고 넘어간다.
	atomic<uint32_t> _inPool;
	atomic<uint32_t> _outPool;
};

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

	void AddRoom(shared_ptr<GameRoom> room);
	void RemoveInvalidRoom();

protected:
	uint64_t _lastRenewRoomTick = 0;
	vector<shared_ptr<GameRoom>> _rooms;
	shared_mutex _roomsLock;
};

class TestMatchManager : public GameManager {
public:
	TestMatchManager() : _ty(GameType::TestMatch), _quota(2), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
	}

	void Push(WatingPlayerData pd) override;
	void Push(vector<WatingPlayerData> pdv) override;
	void RenewMatchQueue();
	void MatchMake() override;
	void MakeRoom(vector<WatingPlayerData>&& pdv) override;

	void Update() override {}

	void StartGame() {}

private:
	GameType _ty = GameType::TestMatch;
	int32_t _quota;
	MatchQueue _matchQueue;
	vector<bool> _excluded;
	uint64_t _lastRenewTick = 0;
};

class PingPongManager : public GameManager {
public:
	PingPongManager() : _ty(GameType::PingPong), _quota(4), _matchQueue(_ty, _quota) {
		_excluded = vector<bool>(_quota);
	}

	void Push(WatingPlayerData pd) override;
	void Push(vector<WatingPlayerData> pdv) override;
	void RenewMatchQueue();
	void MatchMake() override;
	void MakeRoom(vector<WatingPlayerData>&& pdv) override;

	void Update() override {}

	void StartGame() {}

private:
	GameType _ty = GameType::PingPong;
	MatchQueue _matchQueue;
	int32_t _quota = 4;
	vector<bool> _excluded;
	uint64_t _lastRenewTick = 0;
};

extern map<int32_t, shared_ptr<GameManager>> GGameManagers;
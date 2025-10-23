#pragma once
#include "GameType.h"

class GameRoom;

class PlayerSession : public PBSession {
public:
	//TODO:
	void OnConnected();
	void OnDisconnected();
	void OnRecvPacket(unsigned char* buffer, int32_t len) override;

	EVP_PKEY* GetRSAKey();
	void SetAESKey(vector<unsigned char>&& AESKey);
	void SetAESKey(vector<unsigned char>& AESKey);
	vector<unsigned char> GetAESKey();

	void SetSecureLevel(int32_t lv);

	void SetDbid(int32_t dbid);
	int32_t GetDbid() const { return _dbid; }

	void SetElos(int32_t elo1, int32_t elo2, int32_t elo3);
	int32_t GetElo(const int32_t& idx) const;

	void SetMatchingState(const GameType& value);
	bool TryChangeMatchingState(GameType& expected, GameType desired);
	GameType GetMatchingState() const { return _matchingState; }

	void SetRoomIdx(const int32_t roomIdx);
	int32_t GetRoomIdx();

	void SetJoinedRoom(shared_ptr<GameRoom> roomRef);
	shared_ptr<GameRoom> GetJoinedRoom();

	int64_t GetLastKeepAliveTick() const;
	void SetLastKeepAliveTick(const int64_t& tick);

	string GetPlayerId() const;
	void SetPlayerId(const string& playerId);

	static bool IsInvalidPlayerSession(shared_ptr<PlayerSession>& playerSessionRef) {
		if (playerSessionRef == nullptr)
			return true;
		if (playerSessionRef->isConnected() == false)
			return true;
		return false;
	}

private:
	EVP_PKEY* _RSAKey;
	vector<unsigned char> _AESKey;
	int32_t _gameVersion = 0;
	string _playerId;
	int32_t _dbid = 0;
	int32_t _elo1 = 0;
	int32_t _elo2 = 0;
	int32_t _elo3 = 0;

	atomic<GameType> _matchingState = GameType::None;
	weak_ptr<GameRoom> _joinedRoomWRef;
	atomic<int64_t> _lastKeepAliveTick = 0;
	int32_t _roomIdx;
};

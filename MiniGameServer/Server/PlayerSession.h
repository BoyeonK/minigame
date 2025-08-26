#pragma once
#include "GameType.h"

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
	GameType GetMatchingState() const { return _matchingState; }

private:
	EVP_PKEY* _RSAKey;
	vector<unsigned char> _AESKey;
	int32_t _gameVersion = 0;
	int32_t _dbid = 0;
	atomic<GameType> _matchingState = GameType::None;
};

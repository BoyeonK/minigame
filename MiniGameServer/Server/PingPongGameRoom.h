#pragma once
#include "GameRoom.h"
#include "PingPongGameBullet.h"

class PingPongGameRoom : public GameRoom {
public:
	PingPongGameRoom() {
		_ty = GameType::PingPong;
		_points = vector<int32_t>(4, 0);
		_elos = vector<int32_t>(4, 0);
		_dbids = vector<int32_t>(_quota, 0);
		_playerIds = vector<string>(_quota);
	}
	~PingPongGameRoom() {
		cout << "PingPong·ë »ç¸Á" << endl;
	}

	void ReturnToPool();
	void Update() override;

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);

	void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) override;

	void Start();
	void CountdownBeforeStart(int32_t countdown);
	void OnGoingPhase1();
	void OnGoingPhase2();
	void OnGoingPhase3();
	void CountingPhase();
	void CalculateGameResult();
	void UpdateGameResultToDB();
	void UpdateRecords();
	void UpdateElos();
	void EndGame();

	//bool MakeSerializedBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed, S2C_Protocol::S_P_Bullet& outPkt);
	//void MakeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed);
	//void MakeBullets(initializer_list<S2C_Protocol::S_P_Bullet> serializedBullets);
	void MakeBulletsFromPatternMap(const S2C_Protocol::S_P_Bullets& serializedBullets);
	bool SpawnAndInitializeBullet(S2C_Protocol::S_P_Bullet* pSerializedBullet);

	void Handle_CollisionBar(float px, float pz, float speed, int32_t objectId, int32_t playerIdx);
	bool IsVaildCollision(shared_ptr<PingPongGameBullet> bulletRef, float px, float pz, float speed, int32_t playerIdx);
	void Handle_CollisionGoalLine(int32_t playerIdx, int32_t point);
	void Handle_Response_KeepAlive(int32_t playerIdx);

	void RequestPlayerBarPosition();
	void ResponsePlayerBarPosition(int32_t playerIdx, float x, float z);
	void RenewScoreBoard();
	void BroadCastKeepAlive();

	void SendGameState(int32_t playerIdx) override;

private:
	int32_t _quota = 4;
	vector<string> _playerIds;
	vector<int32_t> _dbids;
	vector<int32_t> _elos;
	vector<int32_t> _points;
	vector<int> _winners;

	bool _isUpdateCall = false;
	uint64_t _keepAliveTick = 0;
	S2C_Protocol::S_P_RequestPlayerBarPosition _requestPlayerBarPosPkt;
	S2C_Protocol::S_P_RenewScores _renewScoresPkt;
	S2C_Protocol::S_P_KeepAlive _keepAlivePkt;
	S2C_Protocol::S_GameSceneLoadingProgress _loadingProgressPkt;
	float _ex = 6.4f;
	float _ez = 0;
	float _wx = -6.4f;
	float _wz = 0;
	float _sx = 0;
	float _sz = -6.4f;
	float _nx = 0;
	float _nz = 6.4f;
};
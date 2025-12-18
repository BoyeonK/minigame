#pragma once
#include "GameRoom.h"
#include "TestGameBullet.h"

class RaceRoom : public GameRoom {
public:
	RaceRoom() {
		_ty = GameType::Race;
		_points = vector<int32_t>(_quota, 0);
		_elos = vector<int32_t>(_quota, 0);
		_dbids = vector<int32_t>(_quota, 0);
		_playerIds = vector<string>(_quota);
		_nestedForces = vector<XYZ>(_quota);
		_states = vector<int32_t>(_quota);
		_loadedPlayers = vector<bool>(_quota, false);
		_movementInfos = vector<S2C_Protocol::GameObjectMovementInfo>(_quota);
		_movementAndCollisions = vector<S2C_Protocol::S_R_MovementAndCollision>(_quota);
	}
	~RaceRoom() {
		cout << "·ë »ç¸Á" << endl;
	}

	void ReturnToPool();
	void Update() override;

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);

	void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) override;
	void Start();
	void SendGameState(int32_t playerIdx) override;

	void Countdown();
	void BroadCastCountdownPacket(int32_t count);
	void BroadCastMovementAndCollision();
	void HandleResponseMovementAndCollision(S2C_Protocol::C_R_ResponseMovementAndCollision pkt, int32_t playerIdx);
	void RaceStart();
	void CountingPhase();
	void CalculateGameResult();
	void UpdateGameResultToDB();
	void UpdateRecords();
	void UpdateElos();
	void EndPhase();

	S2C_Protocol::S_R_ResponseState MakeSRResponseState(int32_t playerIdx);

private:
	int32_t _quota = 1;
	bool _isUpdateCall = false;
	vector<string> _playerIds;
	vector<int32_t> _dbids;
	vector<int32_t> _elos;
	vector<int32_t> _points;
	vector<bool> _loadedPlayers;
	vector<XYZ> _nestedForces;
	vector<int32_t> _states;

	S2C_Protocol::S_GameSceneLoadingProgress _loadingProgressPkt;
	S2C_Protocol::S_R_MovementAndCollision _tempMACpkt;
	vector<S2C_Protocol::S_R_MovementAndCollision> _movementAndCollisions;
	vector<S2C_Protocol::GameObjectMovementInfo> _movementInfos;
};

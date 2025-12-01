#pragma once
#include "GameRoom.h"
#include "TestGameBullet.h"

class TestGameRoom : public GameRoom {
public:
	TestGameRoom() {
		_ty = GameType::TestGame;
		_points = vector<int32_t>(_quota, 0);
		_elos = vector<int32_t>(_quota, 0);
		_dbids = vector<int32_t>(_quota, 0);
		_playerIds = vector<string>(_quota);
	}
	~TestGameRoom() {
		cout << "·ë »ç¸Á" << endl;
	}

	void ReturnToPool();
	void Update() override {}

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);

	void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) override;
	void Start();
	void SendGameState(int32_t playerIdx) override;

	shared_ptr<TestGameBullet> MakeTestGameBullet(float x, float y, float z);
	void MakeTestGameBulletAndBroadcast(float x, float y, float z);

	void Phase1();
	void CountingPhase();
	void CalculateGameResult();
	void UpdateGameResultToDB();
	void UpdateRecords();
	void UpdateElos();
	void EndPhase();

	S2C_Protocol::S_TestGameState MakeSTestGameState();

private:
	int32_t _quota = 1;
	bool _isUpdateCall = false;
	vector<string> _playerIds;
	vector<int32_t> _dbids;
	vector<int32_t> _elos;
	vector<int32_t> _points;

	S2C_Protocol::S_GameSceneLoadingProgress _loadingProgressPkt;
};

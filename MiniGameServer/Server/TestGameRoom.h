#pragma once
#include "GameRoom.h"
#include "TestGameBullet.h"

class TestGameRoom : public GameRoom {
public:
	TestGameRoom() {
		_ty = GameType::TestGame;
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
	void EndPhase();
	void CalculateGameResult();

	S2C_Protocol::S_TestGameState MakeSTestGameState();

private:
	int32_t _quota = 1;
};

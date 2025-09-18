#pragma once
#include "GameRoom.h"

class TestGameRoom : public GameRoom {
public:
	TestGameRoom() {
		_ty = GameType::TestMatch;
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

	void MakeTestGameBullet(float x, float y, float z);
	void Phase1();
	void CalculateGameResult();

	S2C_Protocol::S_TestGameState MakeSTestGameState();

private:
	int32_t _quota = 1;
};

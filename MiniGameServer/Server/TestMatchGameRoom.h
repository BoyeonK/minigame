#pragma once
#include "GameRoom.h"

class TestMatchGameRoom : public GameRoom {
public:
	TestMatchGameRoom() {
		_ty = GameType::TestMatch;
	}
	~TestMatchGameRoom() {
		cout << "·ë »ç¸Á" << endl;
	}

	void ReturnToPool();
	void Update() override {}

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);

	void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) override;
	void Start();
	void SendGameState(int32_t playerIdx) override;

	void MakeTestGameBullets();
	void Phase1();
	void Phase2();

	S2C_Protocol::S_TestGameState MakeSTestGameState();

private:
	int32_t _quota = 1;
};

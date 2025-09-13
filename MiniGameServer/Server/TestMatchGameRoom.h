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

	void MakeTestGameBullets();

private:
	int32_t _quota = 1;
};

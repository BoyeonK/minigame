#pragma once
#include "GameRoom.h"

class PingPongGameRoom : public GameRoom {
public:
	PingPongGameRoom() {
		_ty = GameType::PingPong;
	}

	void ReturnToPool();
	void Update() override {}
	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);
	void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) override {}

private:
	int32_t _quota = 4;
};

#pragma once
#include "GameRoom.h"

class PingPongGameRoom : public GameRoom {
public:
	PingPongGameRoom() {
		_ty = GameType::PingPong;
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

	void SendGameState(int32_t playerIdx) override;

private:
	int32_t _quota = 4;
};

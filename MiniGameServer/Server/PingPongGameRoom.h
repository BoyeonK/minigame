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
	void RequestPlayerBarPosition();
	void ResponsePlayerBarPosition(int32_t playerIdx, pair<float, float> barPos);

	void SendGameState(int32_t playerIdx) override;

private:
	int32_t _quota = 4;
	bool _isUpdateCall = false;
	vector<pair<float, float>> _playerBarPositions{ {6.4f, 0}, {-6.4f, 0}, {0, -6.4f}, {0, 6.4f} };
};
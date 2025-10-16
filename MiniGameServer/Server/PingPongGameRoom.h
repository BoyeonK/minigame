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
	void TestPhase1();
	void TestPhase2();

	void MakeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed);
	void Handle_CollisionBar(int32_t objectId, int32_t playerIdx);
	
	void RequestPlayerBarPosition();
	void ResponsePlayerBarPosition(int32_t playerIdx, float x, float z);

	void SendGameState(int32_t playerIdx) override;

private:
	int32_t _quota = 4;
	bool _isUpdateCall = false;
	S2C_Protocol::S_P_RequestPlayerBarPosition _requestPlayerBarPosPkt;

	float _ex = 6.4f;
	float _ez = 0;
	float _wx = -6.4f;
	float _wz = 0;
	float _sx = 0;
	float _sz = -6.4f;
	float _nx = 0;
	float _nz = 6.4f;
};
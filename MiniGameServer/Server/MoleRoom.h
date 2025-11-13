#pragma once
#include "GameRoom.h"

class MoleRoom : public GameRoom {
public:
	MoleRoom() {
		_ty = GameType::Mole;
		_points = vector<int32_t>(2, 0);
		_elos = vector<int32_t>(2, 0);
		_playerIds = vector<string>(2, 0);
	}

	~MoleRoom() {
		cout << "MoleRoom »ç¸Á" << endl;
	}

	void ReturnToPool();
	void Update() override;

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);
	void Start() {};

private:
	int32_t _quota = 2;
	bool _isUpdateCall = false;
	vector<string> _playerIds;
	vector<int32_t> _elos;
	vector<int32_t> _points;
	vector<int> _winners;
};


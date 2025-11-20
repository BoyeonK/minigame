#pragma once
#include "GameRoom.h"

class MoleRoom : public GameRoom {
public:
	MoleRoom() {
		_ty = GameType::Mole;
		_points = vector<int32_t>(_quota, 0);
		_elos = vector<int32_t>(_quota, 0);
		_playerIds = vector<string>(_quota);
		_slotStates = vector<SlotState>(10, SlotState::Yellow);
		_isStunned = vector<bool>(_quota, false);
		_failedResponse.set_isstunned(true);
		_succeedResponse.set_isstunned(false);
	}

	~MoleRoom() {
		cout << "MoleRoom »ç¸Á" << endl;
	}

	void ReturnToPool();
	void Update() override;

	void Init(vector<WatingPlayerData> pdv) override;
	void Init2(vector<WatingPlayerData> pdv);

	void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) override;
	
	void Start();
	void OnGoingPhase1();
	void OnGoingPhase2();

	void CountingPhase() {};
	void CalculateGameResult() {};

	void RenewScoreBoard();

	void SendGameState(int32_t playerIdx) override;

	enum SlotState {
		Red,
		Yellow,
		Green,
	};

	void HitSlot(int32_t playerIdx, int32_t slotNum);
	void SetStun(const int32_t& playerIdx, bool state);
	
private:
	int32_t _quota = 1;
	bool _isUpdateCall = false;
	vector<string> _playerIds;
	vector<int32_t> _elos;
	vector<int32_t> _points;
	vector<int> _winners;
	vector<SlotState> _slotStates;
	vector<bool> _isStunned;

	void HitRed(const int32_t& playerIdx, const int32_t& slotNum);
	void HitYellow(const int32_t& playerIdx);
	void HitGreen(const int32_t& playerIdx, const int32_t& slotNum);
	void SetSlotState(const int32_t& slotNum, SlotState state);
	S2C_Protocol::S_M_ResponseHitSlot _failedResponse;
	S2C_Protocol::S_M_ResponseHitSlot _succeedResponse;
	S2C_Protocol::S_M_SetSlotState _setSlotStatePkt;
	S2C_Protocol::S_GameSceneLoadingProgress _loadingProgressPkt;
};


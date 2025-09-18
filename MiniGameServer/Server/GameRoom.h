#pragma once
#include "MatchQueue.h"
#include "UnityGameObject.h"

class PlayerSession;

class GameRoom : public Actor {
public:
	//BeforeInit : Room�� ���� ������ ���.
	//BeforeStart : Player�� �ε� �� KeepAlive���� ��Ȯ��
	//OnGoing : ������ �������� ���. (���⼭ �� ����ȭ �� ���� ����)
	//Counting : ���� ����. ����� ���� ���������� DB�� �ݿ��ϰ� Room ����.
	enum class GameState {
		BeforeInit,
		BeforeStart,
		OnGoing,
		Counting,
		EndGame,
	};

	void SetRoomId(int32_t roomId);
	int32_t GetRoomId() const;
	virtual void Update() = 0;
	virtual void Init(vector<WatingPlayerData> pdv) = 0;
	GameState GetState() const;
	virtual void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) = 0;
	virtual void SendGameState(int32_t playerIdx) = 0;
	int32_t GenerateUniqueGameObjectId();
	void RegisterGameObject(shared_ptr<UnityGameObject> obj);

protected:
	int32_t _roomId;
	int32_t _nxtObjectId = 0;
	vector<weak_ptr<PlayerSession>> _playerWRefs;
	GameType _ty;
	GameState _state = GameState::BeforeInit;
	int32_t _preparedPlayer = 0;
	vector<shared_ptr<UnityGameObject>> _vecGameObjects;
	unordered_map<int32_t, shared_ptr<UnityGameObject>> _hmGameObjects;
};


#pragma once
#include "MatchQueue.h"
#include "UnityGameObject.h"

class PlayerSession;

class GameRoom : public Actor {
public:
	GameRoom() {
		_nxtObjectId = 0;
		_updateCount = 0;
		_preparedPlayer = 0;
		_nxtObjectId = 0;
		_state = GameState::BeforeInit;
	}

	//BeforeInit : Room이 최초 생성된 경우.
	//BeforeStart : Player의 로딩 및 KeepAlive여부 재확인
	//OnGoing : 게임이 진행중인 경우. (여기서 더 세분화 될 수도 있음)
	//Counting : 게임 종료. 결과에 따른 변동사항을 DB에 반영하고 Room 종료.
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
	uint64_t _lastTimeUpdateCalledTick = 0;

	virtual void Init(vector<WatingPlayerData> pdv) = 0;
	GameState GetState() const;
	virtual void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) = 0;
	virtual void SendGameState(int32_t playerIdx) = 0;
	int32_t GenerateUniqueGameObjectId();
	void RegisterGameObject(const shared_ptr<UnityGameObject>& obj);
	void BroadCast(shared_ptr<SendBuffer> sendBuffer);
	void BroadCastSpawn(const shared_ptr<UnityGameObject>& objRef);
	void BroadCastDespawn(const shared_ptr<UnityGameObject>& objRef);
	double Winrate(int32_t elo1, int32_t elo2);
	int32_t CalculateEloW(int32_t winnerElo, int32_t opponentElo);
	int32_t CalculateEloL(int32_t loserElo, int32_t opponentElo);

protected:
	int32_t _roomId = 0;
	int32_t _nxtObjectId = 0;
	vector<weak_ptr<PlayerSession>> _playerWRefs;
	GameType _ty = GameType::Undefined;
	GameState _state = GameState::BeforeInit;
	int32_t _preparedPlayer = 0;
	uint64_t _updateCount = 0;
	
	//Update문에서의 순회용
	vector<shared_ptr<UnityGameObject>> _vecGameObjects;
	//objectId를 통한 접근용
	unordered_map<int32_t, shared_ptr<UnityGameObject>> _hmGameObjects;
};


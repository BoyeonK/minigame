#pragma once
#include "MatchQueue.h"
#include "UnityGameObject.h"

class PlayerSession;

class GameRoom : public JobQueue {
public:
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

	virtual void Update() = 0;
	virtual void Init(vector<WatingPlayerData> pdv) = 0;
	GameState GetState();
	virtual void UpdateProgressBar(int32_t playerIdx, int32_t progressRate) = 0;
	virtual void SendGameState(int32_t playerIdx) = 0;

protected:
	vector<weak_ptr<PlayerSession>> _playerWRefs;
	GameType _ty;
	GameState _state = GameState::BeforeInit;
	int32_t _preparedPlayer = 0;
	vector<shared_ptr<UnityGameObject>> _vecGameObjects;
	unordered_map<uint32_t, shared_ptr<UnityGameObject>> _hmGameObject;
};

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
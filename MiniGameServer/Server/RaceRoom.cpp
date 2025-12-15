#include "pch.h"
#include "RaceRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "TestGameBullet.h"
#include "RacePlayer.h"

void RaceRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	cout << "TestGame 룸 생성" << endl;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();
		_playerWRefs.push_back(pd.playerSessionWRef);
		if (playerSessionRef == nullptr) {
			ready = false;
			break;
		}
		//각 Session에 KeepAlive패킷을 BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(1);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
	}

	if (ready) {
		//1초 후, (Ping이 1초가 넘는것은, 이상하다.) 모든 패킷으로부터 응답을 받았다면 시작
		PostEventAfter(1000, &RaceRoom::Init2, move(pdv));
	}
	else {
		//유효하지 않은 세션이 있었을 경우, 모두 대기열로 돌려보냄.
		//대기열은 주기적으로 유효하지 않은 PlayerData를 거르도록 설계되어있음.
		GGameManagers[int(_ty)]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

void RaceRoom::Init2(vector<WatingPlayerData> pdv) {
	cout << "Init2" << endl;

	bool canStart = true;
	for (int i = 0; i < _quota; i++) {
		shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef)) {
			canStart = false;
			break;
		}

		int64_t now = ::GetTickCount64();
		int64_t lastTick = playerSessionRef->GetLastKeepAliveTick();
		_elos[i] = playerSessionRef->GetElo(int(_ty));
		_playerIds[i] = playerSessionRef->GetPlayerId();
		_dbids[i] = playerSessionRef->GetDbid();

		if (now - lastTick > 2000) {
			canStart = false;
			break;
		}
	}

	if (canStart) {
		_state = GameState::BeforeStart;
		_preparedPlayer = 0;

		for (int i = 0; i < _quota; i++) {
			shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[i].lock();
			S2C_Protocol::S_MatchmakeCompleted pkt = S2CPacketMaker::MakeSMatchmakeCompleted(int(_ty), _playerIds);
			if (!PlayerSession::IsInvalidPlayerSession(playerSessionRef)) {
				playerSessionRef->SetJoinedRoom(static_pointer_cast<RaceRoom>(shared_from_this()));
				playerSessionRef->SetRoomIdx(i);
				shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
				playerSessionRef->Send(sendBuffer);
			}
		}
	}
	else {
		cout << "게임 시작 불가능." << endl;
		GGameManagers[int(_ty)]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void RaceRoom::Start() {
	if (_state != GameState::BeforeStart)
		return;
	_state = GameState::OnGoing;
	cout << "스타트 함수 실행" << endl;

	for (int i = 0; i < 4; i++) {
		shared_ptr<RacePlayer> runnerRef = { objectPool<RacePlayer>::alloc(-3 + 2 * i, 0, 0), objectPool<RacePlayer>::dealloc };
		runnerRef->SetObjectId(GenerateUniqueGameObjectId());
		RegisterGameObject(runnerRef);
	}

	for (int i = 0; i < _quota; i++)
		_movementAndCollisions[i].set_playerid(i);

	S2C_Protocol::S_GameStarted pkt = S2CPacketMaker::MakeSGameStarted(int(_ty));
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
	PostEventAfter(3000, &RaceRoom::Countdown);
}

void RaceRoom::SendGameState(int32_t playerIdx) {
	if (playerIdx > (_quota - 1) or playerIdx < 0)
		return;

	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	S2C_Protocol::S_R_ResponseState pkt = MakeSRResponseState(playerIdx);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	playerSessionRef->Send(sendBuffer);
	_loadedPlayers[playerIdx] = true;
}

void RaceRoom::Countdown() {
	_isUpdateCall = true;
	BroadCastCountdownPacket(3);
	PostEventAfter(1000, &RaceRoom::BroadCastCountdownPacket, 2);
	PostEventAfter(2000, &RaceRoom::BroadCastCountdownPacket, 1);
	PostEventAfter(3000, &RaceRoom::RaceStart);
}

void RaceRoom::BroadCastCountdownPacket(int32_t count) {
	S2C_Protocol::S_R_SetReadyCommand pkt;
	pkt.set_countdown(count);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void RaceRoom::BroadCastMovementAndCollision() {
	_tempMACpkt.clear_movementinfos();
	for (int i = 0; i < _quota; i++) {
		_tempMACpkt.add_movementinfos()->CopyFrom(_movementInfos[1]);
	}
	
	for (int i = 0; i < _quota; i++) {
		shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		_movementAndCollisions[i].mutable_collisionnestedforce()->CopyFrom(_nestedForces[i].Serialize());
		_movementAndCollisions[i].mutable_movementinfos()->CopyFrom(_tempMACpkt.movementinfos());

		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_movementAndCollisions[i]);
		playerSessionRef->Send(sendBuffer);
	}

	for (int i = 0; i < _quota; i++) {
		_movementAndCollisions[i].clear_movementinfos();
		_movementAndCollisions[i].clear_collisionnestedforce();
	}
}

void RaceRoom::HandleResponseMovementAndCollision(S2C_Protocol::C_R_ResponseMovementAndCollision pkt, int32_t playerIdx) {
	vector<int32_t> collisionObjIds(pkt.objectids_size());
	for (int i = 0; i < pkt.objectids_size(); i++)
		collisionObjIds[i] = pkt.objectids().Get(i);

	//FM대로 하면, position이 정당한 움직임인지 변위 체크 필요할듯.
	_positions[playerIdx].DeserializeFrom(pkt.movementinfo().position());
	_fronts[playerIdx].DeserializeFrom(pkt.movementinfo().front());
	_velocitys[playerIdx].DeserializeFrom(pkt.movementinfo().velocity());
	_states[playerIdx] = pkt.movementinfo().state();
}

void RaceRoom::RaceStart() {
	PostEventAfter(60000, &RaceRoom::CountingPhase);
}

void RaceRoom::CountingPhase() {
	cout << "Calculating" << endl;
	_state = GameState::Counting;
	_isUpdateCall = false;
	CalculateGameResult();
}

void RaceRoom::CalculateGameResult() {
	for (int i = 0; i < _quota; i++) {
		auto playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		playerSessionRef->SetJoinedRoom(nullptr);
		playerSessionRef->SetMatchingState(GameType::None);
	}

	S2C_Protocol::S_EndGame pkt = S2CPacketMaker::MakeSEndGame();
	pkt.set_gameid(int(_ty));
	S2C_Protocol::S_TestGameResult* pTestGameResult = pkt.mutable_testgameresult();
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);

	PostEvent(&RaceRoom::EndPhase);
}

void RaceRoom::UpdateGameResultToDB() {

}

void RaceRoom::UpdateRecords() {

}

void RaceRoom::UpdateElos() {

}

void RaceRoom::EndPhase() {
	_vecGameObjects.clear();
	_hmGameObjects.clear();
	_playerIds.clear();
	_dbids.clear();
	_elos.clear();
	_points.clear();
	_state = GameState::EndGame;
}

S2C_Protocol::S_R_ResponseState RaceRoom::MakeSRResponseState(int32_t playerIdx) {
	S2C_Protocol::S_R_ResponseState pkt;
	pkt.set_playerid(playerIdx);
	for (auto& goRef : _vecGameObjects) {
		goRef->SerializeObject(pkt.add_objects());
	}
	return pkt;
}

void RaceRoom::ReturnToPool() {
	objectPool<RaceRoom>::dealloc(this);
}

void RaceRoom::Update() {
	if (!_isUpdateCall)
		return;

	_updateCount++;
	BroadCastMovementAndCollision();
}

void RaceRoom::UpdateProgressBar(int32_t playerIdx, int32_t progressRate) {
	cout << "업데이트 프로그레스 바" << endl;
	cout << "_quota : " << _quota << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}

	_loadingProgressPkt.set_playeridx(playerIdx);
	_loadingProgressPkt.set_persentage(progressRate);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_loadingProgressPkt);
	BroadCast(sendBuffer);

	if (_preparedPlayer == _quota) {
		Start();
	}
}
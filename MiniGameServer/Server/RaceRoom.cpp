#include "pch.h"
#include "RaceRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
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
		shared_ptr<RacePlayer> runnerRef = { objectPool<RacePlayer>::alloc(0, 0, -3 + 2 * i), objectPool<RacePlayer>::dealloc };
		runnerRef->SetObjectId(GenerateUniqueGameObjectId());
		RegisterGameObject(runnerRef);
	}

	S2C_Protocol::XYZ front;
	front.set_x(0); front.set_y(0); front.set_z(1);
	S2C_Protocol::XYZ position;
	position.set_y(0); position.set_z(0);
	S2C_Protocol::XYZ velocity;
	velocity.set_x(0); velocity.set_y(0); velocity.set_z(1);

	S2C_Protocol::XYZ nestedForce;
	nestedForce.set_x(0); nestedForce.set_y(0); nestedForce.set_z(0);
	
	for (int i = 0; i < _quota; i++) {
		_movementAndCollisions[i].set_playerid(i);
		_movementAndCollisions[i].mutable_collisionnestedforce()->CopyFrom(nestedForce);

		_movementInfos[i].set_objectid(i);
		_movementInfos[i].mutable_front()->CopyFrom(front);
		position.set_x(-3 + 2 * i);
		_movementInfos[i].mutable_position()->CopyFrom(position);
		_movementInfos[i].mutable_velocity()->CopyFrom(velocity);
		_movementInfos[i].set_state(0);
	}

	S2C_Protocol::S_GameStarted pkt = S2CPacketMaker::MakeSGameStarted(int(_ty));
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
	PostEventAfter(4000, &RaceRoom::Countdown);
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
		_tempMACpkt.add_movementinfos()->CopyFrom(_movementInfos[i]);
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
		_movementAndCollisions[i].clear_collisionnestedforce();
		_nestedForces[i] = _zeroXYZ;
	}
}

void RaceRoom::HandleResponseMovementAndCollision(S2C_Protocol::C_R_ResponseMovementAndCollision pkt, int32_t playerIdx) {
	vector<int32_t> collisionObjIds(pkt.objectids_size());
	for (int i = 0; i < pkt.objectids_size(); i++)
		collisionObjIds[i] = pkt.objectids().Get(i);

	//FM대로 하면, position이 정당한 움직임인지 변위 체크 필요할듯.
	/* 아래의 요소를 현재의 요소와 비교하여 부정행위인지 확인
	pkt.movementinfo().position();
	pkt.movementinfo().front();
	pkt.movementinfo().velocity();
	pkt.movementinfo().state();
	*/

	_movementInfos[playerIdx] = pkt.movementinfo();
	_movementInfos[playerIdx].set_objectid(playerIdx);
}

void RaceRoom::HandleArriveInNextLine(int32_t playerIdx, int32_t lineId) {
	if (lineId == (_stages[playerIdx] + 1) and lineId <= 3) {
		_stages[playerIdx] = lineId;

		S2C_Protocol::S_R_ResponseArriveInNextLine pkt;
		pkt.set_lineid(lineId);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);

		shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			return;

		playerSessionRef->Send(sendBuffer);

		if (lineId == 3) {
			WinnerDecided(playerIdx);
		}
	}
}

void RaceRoom::HandleFallDown(int32_t playerIdx) {
	int32_t lineId = _stages[playerIdx];

	S2C_Protocol::S_R_ResponseFallDown pkt;
	S2C_Protocol::XYZ* position = pkt.mutable_position();
	if (lineId == 0) {
		position->set_x(0);
		position->set_y(2);
		position->set_z(0);
	}
	else if (lineId == 1) {
		position->set_x(30);
		position->set_y(2);
		position->set_z(0);
	}
	else if (lineId == 2) {
		position->set_x(52);
		position->set_y(5.75);
		position->set_z(0);
	}

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	if (sendBuffer == nullptr)
		return;

	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	playerSessionRef->Send(sendBuffer);
}

void RaceRoom::RaceStart() {
	BroadCastCountdownPacket(0);
	_raceStartTick = GetTickCount64();
	PostEventAfter(120000, &RaceRoom::WinnerDecided, -1);
}

void RaceRoom::OperateObstacle(int32_t obstacleId, int32_t operateId) {
	S2C_Protocol::S_R_TriggerObstacle pkt;
	pkt.set_obstacleid(obstacleId);
	pkt.set_triggerid(operateId);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void RaceRoom::OperateObstacles() {
	if (_updateCount == 0) {
		OperateObstacle(0, 0);
		OperateObstacle(1, 1);
	}
	else if (_updateCount == 25) {
		OperateObstacle(2, 0);
		OperateObstacle(3, 1);
	}
	else if (_updateCount == 50) {
		OperateObstacle(0, 1);
		OperateObstacle(1, 0);
	}
	else if (_updateCount == 75) {
		OperateObstacle(2, 1);
		OperateObstacle(3, 0);
	}
}

void RaceRoom::WinnerDecided(int32_t winnerIdx) {
	if (_state != GameState::OnGoing)
		return;

	_raceEndTick = GetTickCount64();
	_winnerIdx = winnerIdx;

	_state = GameState::Counting;
	CountingPhase();
}

void RaceRoom::CountingPhase() {
	cout << "Calculating" << endl;
	_isUpdateCall = false;
	CalculateGameResult();
}

void RaceRoom::CalculateGameResult() {
	int32_t point = (120000 - (_raceEndTick - _raceStartTick)) / 100;

	if (_winnerIdx != -1)
		_points[_winnerIdx] = point >= 0 ? point : 0;

	for (int i = 0; i < _quota; i++) {
		auto playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		S2C_Protocol::S_R_Result pkt;

		if (i == _winnerIdx)
			pkt.set_iswinner(true);
		else
			pkt.set_iswinner(false);
		pkt.set_winneridx(_winnerIdx);

		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);

		playerSessionRef->SetJoinedRoom(nullptr);
		playerSessionRef->SetMatchingState(GameType::None);
	}

	PostEvent(&RaceRoom::UpdateGameResultToDB);
}

void RaceRoom::UpdateGameResultToDB() {
	UpdateRecords();
	UpdateElos();
	PostEvent(&RaceRoom::EndPhase);
}

void RaceRoom::UpdateRecords() {
	for (int i = 0; i < _quota; i++) {
		auto playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		int32_t dbid = playerSessionRef->GetDbid();
		if (_points[i] > playerSessionRef->GetPersonalRecord(int(_ty))) {
			DBManager->S2D_UpdatePersonalRecord(playerSessionRef, dbid, int(_ty), _points[i]);
		}
		GGameManagers[int(_ty)]->CompareAndRenewPublicRecord(dbid, _points[i]);
	}
}

void RaceRoom::UpdateElos() {
	if (_winnerIdx == -1)
		return;

	int32_t bestLosersElo = 0;
	int32_t winnerElo = _elos[_winnerIdx];

	for (int i = 0; i < _quota; i++) {
		if (i == _winnerIdx)
			continue;
		if (_elos[i] > bestLosersElo)
			bestLosersElo = _elos[i];
	}

	if (bestLosersElo != 0) {
		for (int i = 0; i < _quota; i++) {
			int32_t calculatedElo = -1;
			if (i == _winnerIdx)
				calculatedElo = CalculateEloW(_elos[i], bestLosersElo);
			else
				calculatedElo = CalculateEloL(_elos[i], _elos[_winnerIdx]);

			if (calculatedElo == -1)
				continue;

			int32_t dbid = _dbids[i];
			DBManager->S2D_UpdateElo(dbid, int(_ty), calculatedElo);
		}
	}
	else {
		cout << "너무 많은 플레이어가 이탈했거나, 정상적인 진행이 되지 않은 게임" << endl;
	}
}

void RaceRoom::EndPhase() {
	_vecGameObjects.clear();
	_hmGameObjects.clear();
	_playerIds.clear();
	_dbids.clear();
	_elos.clear();
	_points.clear();
	_nestedForces.clear();
	_states.clear();
	_stages.clear();
	_loadedPlayers.clear();
	_movementInfos.clear();
	_movementAndCollisions.clear();
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
	if (_updateCount >= 100)
		_updateCount = 0;

	OperateObstacles();
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
#include "pch.h"
#include "MoleRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void MoleRoom::ReturnToPool() {
	objectPool<MoleRoom>::dealloc(this);
}

void MoleRoom::Update() {
	if (!_isUpdateCall)
		return;

	RenewScoreBoard();
}

void MoleRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	cout << "Mole 룸 생성" << endl;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();
		_playerWRefs.push_back(pd.playerSessionWRef);
		if (playerSessionRef == nullptr) {
			ready = false;
			break;
		}
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(int(_ty));
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
	}

	if (ready) {
		PostEventAfter(1000, &MoleRoom::Init2, move(pdv));
	}
	else {
		GGameManagers[int(_ty)]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

void MoleRoom::Init2(vector<WatingPlayerData> pdv) {
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
				playerSessionRef->SetJoinedRoom(static_pointer_cast<MoleRoom>(shared_from_this()));
				playerSessionRef->SetRoomIdx(i);
				shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
				playerSessionRef->Send(sendBuffer);
			}
		}
		PostEventAfter(30000, &MoleRoom::Start);
	}
	else {
		cout << "게임 시작 불가능." << endl;
		GGameManagers[int(_ty)]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void MoleRoom::UpdateProgressBar(int32_t playerIdx, int32_t progressRate) {
	cout << "업데이트 프로그레스 바" << endl;
	cout << "_quota : " << _quota << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}
	cout << "_preparedPlayer : " << _preparedPlayer << endl;

	_loadingProgressPkt.set_playeridx(playerIdx);
	_loadingProgressPkt.set_persentage(progressRate);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_loadingProgressPkt);
	BroadCast(sendBuffer);

	if (_preparedPlayer == _quota) {
		Start();
	}
}

void MoleRoom::Start() {
	if (_state != GameState::BeforeStart)
		return;
	_state = GameState::OnGoing;

	cout << "스타트 함수 실행" << endl;
	S2C_Protocol::S_GameStarted pkt = S2CPacketMaker::MakeSGameStarted(int(_ty));
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);

	PostEventAfter(2000, &MoleRoom::CountdownBeforeStart, 3);
	PostEventAfter(3000, &MoleRoom::CountdownBeforeStart, 2);
	PostEventAfter(4000, &MoleRoom::CountdownBeforeStart, 1);
	PostEventAfter(5000, &MoleRoom::OnGoingPhase1);
}

void MoleRoom::CountdownBeforeStart(int32_t count) {
	S2C_Protocol::S_M_ReadyForStart pkt;
	pkt.set_countdown(count);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void MoleRoom::OnGoingPhase1() {
	_isUpdateCall = true;
	CountdownBeforeStart(0);

	int delay = 0;

	for (int i = 1; i <= MAX_WAVE; i++) {
		int32_t isRedExcluded = rand() % 3;
		int32_t gSlot = rand() % SLOT_COUNT + 1;

		int32_t randDelay = rand() % MAX_RAND_DELAY;
		int32_t baseDelay = START_DELAY - i * DELAY_DECREMENT + randDelay;
		delay += baseDelay;

		if (isRedExcluded != 0)
			PostEventAfter(delay, &MoleRoom::SetSlotState, move(gSlot), SlotState::Green);
		else {
			int32_t rSlot = rand() % SLOT_COUNT + 1;
			while (gSlot == rSlot)
				rSlot = rand() % SLOT_COUNT + 1;
			PostEventAfter(delay, &MoleRoom::SetSlotState, move(gSlot), SlotState::Green);
			PostEventAfter(delay, &MoleRoom::SetSlotState, move(rSlot), SlotState::Red);
		}
	}
	PostEventAfter(delay + 2000, &MoleRoom::CountingPhase);
}

void MoleRoom::OnGoingPhase2() {

}

void MoleRoom::RenewScoreBoard() {
	_renewScoresPkt.Clear();

	for (auto& point : _points) {
		_renewScoresPkt.add_scores(point);
	}

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_renewScoresPkt);
	BroadCast(sendBuffer);
}

void MoleRoom::SendGameState(int32_t playerIdx) {
	if (playerIdx > (_quota - 1))
		return;

	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	S2C_Protocol::S_M_State pkt;
	pkt.set_playerid(playerIdx);

	for (auto& playerId : _playerIds) pkt.add_ids(playerId);

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	playerSessionRef->Send(sendBuffer);
}

void MoleRoom::HitSlot(int32_t playerIdx, int32_t slotIdx) {
	if (_isStunned[playerIdx])
		return;

	SlotState ss = _slotStates[slotIdx];
	switch (ss) {
	case SlotState::Red:
		HitRed(playerIdx, slotIdx);
		break;
	case SlotState::Yellow:
		HitYellow(playerIdx);
		break;
	case SlotState::Green:
		HitGreen(playerIdx, slotIdx);
		break;
	default:
		break;
	}
}

void MoleRoom::SetStun(const int32_t& playerIdx, bool state) {
	_isStunned[playerIdx] = state;
}

void MoleRoom::HitRed(const int32_t& playerIdx, const int32_t& slotNum) {
	_points[playerIdx] = _points[playerIdx] - 15;
	SetSlotState(slotNum, SlotState::Yellow);
	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	SetStun(playerIdx, true);
	PostEventAfter(1000, &MoleRoom::SetStun, playerIdx, false);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_failedResponse);
	playerSessionRef->Send(sendBuffer);
}

void MoleRoom::HitYellow(const int32_t& playerIdx) {
	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	SetStun(playerIdx, true);
	PostEventAfter(1000, &MoleRoom::SetStun, playerIdx, false);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_failedResponse);
	playerSessionRef->Send(sendBuffer);
}

void MoleRoom::HitGreen(const int32_t& playerIdx, const int32_t& slotNum) {
	_points[playerIdx] = _points[playerIdx] + 10;
	SetSlotState(slotNum, SlotState::Yellow);
	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_succeedResponse);
	playerSessionRef->Send(sendBuffer);
}

void MoleRoom::SetSlotState(int32_t slotIdx, SlotState state) {
	if (_slotStates[slotIdx] == state)
		return;

	if (state == Red && _slotStates[slotIdx] == Green)
		return;

	if (state == Green && _slotStates[slotIdx] == Red)
		return;

	_slotStates[slotIdx] = state;
	_setSlotStatePkt.set_slotidx(slotIdx);
	_setSlotStatePkt.set_state(int(state));

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_setSlotStatePkt);
	BroadCast(sendBuffer);
}

void MoleRoom::CountingPhase() {
	cout << "Calculating" << endl;
	_state = GameState::Counting;
	_isUpdateCall = false;
	CalculateGameResult();
}

void MoleRoom::CalculateGameResult() {
	int32_t mxm = -1000;
	for (int i = 0; i < _quota; i++) {
		if (_points[i] > mxm) {
			_winners.clear();
			mxm = _points[i];
			_winners.push_back(i);
		}
		else if (_points[i] == mxm) {
			_winners.push_back(i);
		}
	}

	S2C_Protocol::S_M_Result baseResultPkt;

	for (int i = 0; i < _quota; i++) {
		baseResultPkt.add_scores(_points[i]);
	}

	for (int i = 0; i < _quota; i++) {
		auto playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		bool isWinner = find(_winners.begin(), _winners.end(), i) != _winners.end();

		S2C_Protocol::S_M_Result playerResultPkt = baseResultPkt;
		playerResultPkt.set_iswinner(isWinner);

		playerSessionRef->SetJoinedRoom(nullptr);
		playerSessionRef->SetMatchingState(GameType::None);

		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(playerResultPkt);
		playerSessionRef->Send(sendBuffer);
	}

	PostEvent(&MoleRoom::UpdateGameResultToDB);
}

void MoleRoom::UpdateGameResultToDB() {
	UpdateRecords();
	UpdateElos();
	PostEvent(&MoleRoom::EndGame);
}

void MoleRoom::UpdateRecords() {
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

void MoleRoom::UpdateElos() {
	int32_t bestLosersElo = 0;
	int32_t worstWinnerElo = 3000;

	for (int i = 0; i < _quota; i++) {
		bool isWinner = find(_winners.begin(), _winners.end(), i) != _winners.end();
		if (isWinner) {
			if (worstWinnerElo > _elos[i])
				worstWinnerElo = _elos[i];
		}
		else {
			if (_elos[i] > bestLosersElo)
				bestLosersElo = _elos[i];
		}
	}

	if (!(worstWinnerElo == 3000 || bestLosersElo == 0)) {
		for (int i = 0; i < _quota; i++) {
			bool isWinner = find(_winners.begin(), _winners.end(), i) != _winners.end();
			int32_t calculatedElo = -1;
			if (isWinner)
				calculatedElo = CalculateEloW(_elos[i], bestLosersElo);
			else
				calculatedElo = CalculateEloL(_elos[i], worstWinnerElo);

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

void MoleRoom::EndGame() {
	_vecGameObjects.clear();
	_hmGameObjects.clear();
	_playerIds.clear();
	_dbids.clear();
	_elos.clear();
	_points.clear();
	_winners.clear();
	_slotStates.clear();
	_isStunned.clear();
	_state = GameState::EndGame;
}
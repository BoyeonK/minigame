#include "pch.h"
#include "MoleRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void MoleRoom::ReturnToPool() {
	objectPool<MoleRoom>::dealloc(this);
}

void MoleRoom::Update() {

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
		//30초 뒤에는 강제로 시작해버려
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

	PostEventAfter(5000, &MoleRoom::OnGoingPhase1);
}

void MoleRoom::OnGoingPhase1() {
	PostEventAfter(1000, &MoleRoom::SetSlotState, 1, SlotState::Green);
	PostEventAfter(2000, &MoleRoom::SetSlotState, 2, SlotState::Green);
	PostEventAfter(3000, &MoleRoom::SetSlotState, 3, SlotState::Green);
	PostEventAfter(4000, &MoleRoom::SetSlotState, 4, SlotState::Green);
	PostEventAfter(5000, &MoleRoom::SetSlotState, 5, SlotState::Green);
	PostEventAfter(6000, &MoleRoom::SetSlotState, 6, SlotState::Green);
	PostEventAfter(7000, &MoleRoom::SetSlotState, 7, SlotState::Green);
	PostEventAfter(8000, &MoleRoom::SetSlotState, 8, SlotState::Green);
	PostEventAfter(9000, &MoleRoom::SetSlotState, 9, SlotState::Green);
}

void MoleRoom::OnGoingPhase2() {

}

void MoleRoom::RenewScoreBoard() {
	if (_updateCount % 10 != 0)
		return;

	S2C_Protocol::S_M_RenewScores pkt;
	for (auto& point : _points)	pkt.add_scores(point);

	//test
	pkt.add_scores(123);

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
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

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_succeedResponse);
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

	_slotStates[slotIdx] = state;
	_setSlotStatePkt.set_slotidx(slotIdx);
	_setSlotStatePkt.set_state(int(state));

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_setSlotStatePkt);
	BroadCast(sendBuffer);
}

#include "pch.h"
#include "MoleRoom.h"
#include "S2CPacketMaker.h"
#include "S2CPacketHandler.h"

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
	for (auto& playerSessionWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef)) {
			canStart = false;
			break;
		}
		int64_t now = ::GetTickCount64();
		int64_t lastTick = playerSessionRef->GetLastKeepAliveTick();

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
			S2C_Protocol::S_MatchmakeCompleted pkt = S2CPacketMaker::MakeSMatchmakeCompleted(int(_ty));
			if (!PlayerSession::IsInvalidPlayerSession(playerSessionRef)) {
				playerSessionRef->SetJoinedRoom(static_pointer_cast<MoleRoom>(shared_from_this()));
				_elos[i] = playerSessionRef->GetElo(int(_ty));
				_playerIds[i] = playerSessionRef->GetPlayerId();
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

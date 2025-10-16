#include "pch.h"
#include "PingPongGameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "PingPongGameBullet.h"

void PingPongGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	cout << "PingPong 룸 생성" << endl;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();
		_playerWRefs.push_back(pd.playerSessionWRef);
		if (playerSessionRef == nullptr) {
			ready = false;
			break;
		}
		//각 Session에 KeepAlive패킷을 BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(int(_ty));
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
	}

	if (ready) {
		//1초 후, (Ping이 1초가 넘는것은, 이상하다.) 모든 패킷으로부터 응답을 받았다면 시작
		PostEventAfter(1000, &PingPongGameRoom::Init2, move(pdv));
	}
	else {
		//유효하지 않은 세션이 있었을 경우, 모두 대기열로 돌려보냄.
		//대기열은 주기적으로 유효하지 않은 PlayerData를 거르도록 설계되어있음.
		GGameManagers[int(_ty)]->Push(move(pdv));
		_state = GameState::EndGame;
	}
}

void PingPongGameRoom::Init2(vector<WatingPlayerData> pdv) {
	cout << "Init2" << endl;

	bool canStart = true;
	for (auto& playerSessionWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		//유효하지 않은 플레이어가 있는 경우, 모두 대기열로 돌려보냄.
		if (playerSessionRef == nullptr) {
			canStart = false;
			break;
		}
		int64_t now = ::GetTickCount64();
		int64_t lastTick = playerSessionRef->GetLastKeepAliveTick();

		//C_KeepAlive Handler함수에 의해서 lastTick이 변화하지 않았다면,
		//유효하지 않은 플레이어로 간주하고, 모두 대기열로 돌려보냄.
		if (now - lastTick > 2000) {
			canStart = false;
			break;
		}
	}

	if (canStart) {
		//이제는 정말 게임을 진행할 것임.
		//지금부터 연결상태가 좋지 않으면 플레이어 책임으로 간주.
		//플레이어의 게임종료 등의 이유로 세션이 유효하지 않더라도, 진행 가능한 방식으로 코드를 작성해야 함.
		_state = GameState::BeforeStart;
		_preparedPlayer = 0;
		int playerIdx = 0;
		for (auto& playerSessionWRef : _playerWRefs) {
			shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
			S2C_Protocol::S_MatchmakeCompleted pkt = S2CPacketMaker::MakeSMatchmakeCompleted(int(_ty));
			if (playerSessionRef != nullptr) {
				playerSessionRef->SetJoinedRoom(static_pointer_cast<PingPongGameRoom>(shared_from_this()));
				playerSessionRef->SetRoomIdx(playerIdx++);
				shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
				playerSessionRef->Send(sendBuffer);
			}
		}
		//30초 뒤에는 강제로 시작해버려
		PostEventAfter(30000, &PingPongGameRoom::Start);
	}
	else {
		cout << "게임 시작 불가능." << endl;
		GGameManagers[int(_ty)]->Push(pdv);
		_state = GameState::EndGame;
	}
}

void PingPongGameRoom::UpdateProgressBar(int32_t playerIdx, int32_t progressRate) {
	cout << "업데이트 프로그레스 바" << endl;
	cout << "_quota : " << _quota << endl;
	if (progressRate == 100) {
		_preparedPlayer += 1;
	}
	cout << "_preparedPlayer : " << _preparedPlayer << endl;
	//TODO : 로딩 진행상황 전파

	if (_preparedPlayer == _quota) {
		Start();
	}
}

void PingPongGameRoom::Start() {
	if (_state != GameState::BeforeStart)
		return;
	_state = GameState::OnGoing;

	cout << "스타트 함수 실행" << endl;
	S2C_Protocol::S_GameStarted pkt = S2CPacketMaker::MakeSGameStarted(int(_ty));
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);

	PostEventAfter(5000, &PingPongGameRoom::TestPhase1);
}

void PingPongGameRoom::TestPhase1() {
	_isUpdateCall = true;
	PostEventAfter(1000, &PingPongGameRoom::TestPhase2);
}

void PingPongGameRoom::TestPhase2() {
	MakeBullet(0, 0, 0, 1, 1, 1);
}

void PingPongGameRoom::MakeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed) {
	//1. Bullet의 생성.
	shared_ptr<PingPongGameBullet> bulletRef = nullptr;
	switch (bulletType) {
	case 0: {
		shared_ptr<PingPongGameBulletRed> redBullet = PingPongGameBulletRed::NewTestGameBullet(px, 0.2f, pz);
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(redBullet);
		break;
	}
	case 1: {
		shared_ptr<PingPongGameBulletBlue> blueBullet = PingPongGameBulletBlue::NewTestGameBullet(px, 0.2f, pz);
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(blueBullet);
		break;
	}
	case 2: {
		shared_ptr<PingPongGameBulletPupple> puppleBullet = PingPongGameBulletPupple::NewTestGameBullet(px, 0.2f, pz);
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(puppleBullet);
		break;
	}
	default:
		break;
	}
	bulletRef->SetObjectId(GenerateUniqueGameObjectId());
	bulletRef->SetMoveVector(sx, sz, speed);
	bulletRef->UpdateTick(::GetTickCount64());
	RegisterGameObject(bulletRef);

	//Extra. 해당 방향으로 bullet이 직진했을 경우, 감점이 될 플레이어를 계산.
	//추후, 이 bullet에 대한 그 플레이어의 충돌판정 없이, 그 플레이어의 클라이언트로부터 해당 bullet에 대한 GoalLine판정이 들어오지 않는다면 부정행위 의심.


	//2. 해당 Bullet의 생성을 Bullet의 정보를 담아 직렬화
	S2C_Protocol::S_P_Bullet pkt;

	S2C_Protocol::UnityGameObject* bullet_ptr = pkt.mutable_bullet();
	bullet_ptr->set_objectid(bulletRef->GetObjectId());
	bullet_ptr->set_objecttype(bulletRef->GetObjectTypeInteger());
	S2C_Protocol::XYZ* pos_ptr = bullet_ptr->mutable_position();
	pos_ptr->set_x(px);
	pos_ptr->set_z(pz);

	S2C_Protocol::XYZ* moveDir_ptr = pkt.mutable_movedir();
	moveDir_ptr->set_x(sx);
	moveDir_ptr->set_z(sz);

	pkt.set_speed(speed);
	pkt.set_lastcollider(-1);

	//3. 전송.
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void PingPongGameRoom::Handle_CollisionBar(int32_t objectId, int32_t playerIdx) {

}

void PingPongGameRoom::RequestPlayerBarPosition() {
	_requestPlayerBarPosPkt.set_ex(_ex);
	_requestPlayerBarPosPkt.set_ez(_ez);
	_requestPlayerBarPosPkt.set_wx(_wx);
	_requestPlayerBarPosPkt.set_wz(_wz);
	_requestPlayerBarPosPkt.set_sx(_sx);
	_requestPlayerBarPosPkt.set_sz(_sz);
	_requestPlayerBarPosPkt.set_nx(_nx);
	_requestPlayerBarPosPkt.set_nz(_nz);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_requestPlayerBarPosPkt);
	BroadCast(sendBuffer);
}

void PingPongGameRoom::ResponsePlayerBarPosition(int32_t playerIdx, float x, float z) {
	switch (playerIdx) {
	case(0):
		_ex = x;
		_ez = z;
		break;
	case(1):
		_wx = x;
		_wz = z;
		break;
	case(2):
		_sx = x;
		_sz = z;
		break;
	case(3):
		_nx = x;
		_nz = z;
		break;
	default:
		break;
	}
}

void PingPongGameRoom::SendGameState(int32_t playerIdx) {
	if (playerIdx > (_quota - 1))
		return;

	shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[playerIdx].lock();
	if (playerSessionRef == nullptr)
		return;

	//0:동 1:서 2:남 3:북
	S2C_Protocol::S_P_State pkt;
	pkt.set_playerid(playerIdx);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	playerSessionRef->Send(sendBuffer);
}

void PingPongGameRoom::ReturnToPool() {
	objectPool<PingPongGameRoom>::dealloc(this);
}

void PingPongGameRoom::Update() {
	if (!_isUpdateCall)
		return;

	RequestPlayerBarPosition();
}

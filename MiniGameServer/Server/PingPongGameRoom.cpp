#include "pch.h"
#include "PingPongGameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"
#include "PingPongGameBullet.h"
#include <random>

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

		for (int i = 0; i < _quota; i++) {
			shared_ptr<PlayerSession> playerSessionRef = _playerWRefs[i].lock();
			S2C_Protocol::S_MatchmakeCompleted pkt = S2CPacketMaker::MakeSMatchmakeCompleted(int(_ty));
			if (!PlayerSession::IsInvalidPlayerSession(playerSessionRef)) {
				playerSessionRef->SetJoinedRoom(static_pointer_cast<PingPongGameRoom>(shared_from_this()));
				playerSessionRef->SetRoomIdx(i);
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
	_loadingProgressPkt.set_playeridx(playerIdx);
	_loadingProgressPkt.set_persentage(progressRate);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(_loadingProgressPkt);
	BroadCast(sendBuffer);

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

	PostEventAfter(5000, &PingPongGameRoom::OnGoingPhase1);
}

void PingPongGameRoom::OnGoingPhase1() {
	cout << "OnGoingPhase1, 테스트를 위하여 2, 3페이즈를 건너뜀" << endl;
	_isUpdateCall = true;
	vector<int> selectedNums(10);
	uniform_int_distribution<int> dis(0, 5);
	for (int i = 0; i <= 9; i++) {
		const S2C_Protocol::S_P_Bullets bullets = pPingPongManager->easyPatterns[dis(LRanGen)];
		PostEventAfter(2000 * i, &PingPongGameRoom::MakeBulletsFromPatternMap, bullets);
	}
	PostEventAfter(26000, &PingPongGameRoom::CountingPhase);
}

void PingPongGameRoom::OnGoingPhase2() {
	cout << "OnGoingPhase2" << endl;
	vector<int> selectedNums(10);
	uniform_int_distribution<int> dis(0, 5);

	for (int i = 1; i <= 5; i++) {
		const S2C_Protocol::S_P_Bullets bullets = pPingPongManager->easyPatterns[dis(LRanGen)];
		PostEventAfter(4000 * i - 2000, &PingPongGameRoom::MakeBulletsFromPatternMap, bullets);
	}

	for (int i = 1; i <= 5; i++) {
		const S2C_Protocol::S_P_Bullets bullets = pPingPongManager->mediumPatterns[dis(LRanGen)];
		PostEventAfter(4000 * i, &PingPongGameRoom::MakeBulletsFromPatternMap, bullets);
	}
	PostEventAfter(26000, &PingPongGameRoom::OnGoingPhase3);
}

void PingPongGameRoom::OnGoingPhase3() {
	cout << "OnGoingPhase3" << endl;
}

void PingPongGameRoom::CountingPhase() {
	cout << "Calculating" << endl;
	_state = GameState::Counting;
	_isUpdateCall = false;
	CalculateGameResult();
}

void PingPongGameRoom::CalculateGameResult() {
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

	S2C_Protocol::S_P_Result baseResultPkt;

	for (int i = 0; i < _quota; i++) {
		baseResultPkt.add_scores(_points[i]);
	}

	for (int i = 0; i < _quota; i++) {
		auto playerSessionRef = _playerWRefs[i].lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		bool isWinner = find(_winners.begin(), _winners.end(), i) != _winners.end();

		S2C_Protocol::S_P_Result playerResultPkt = baseResultPkt;
		playerResultPkt.set_iswinner(isWinner);

		playerSessionRef->SetJoinedRoom(nullptr);
		playerSessionRef->SetMatchingState(GameType::None);

		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(playerResultPkt);
		playerSessionRef->Send(sendBuffer);
	}

	PostEvent(&PingPongGameRoom::UpdateGameResultToDB);
}

void PingPongGameRoom::UpdateGameResultToDB() {
	UpdateRecords();
	UpdateElos();
	PostEvent(&PingPongGameRoom::EndGame);
}

void PingPongGameRoom::UpdateRecords() {
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

void PingPongGameRoom::UpdateElos() {
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

void PingPongGameRoom::EndGame() {
	//TODO: 모든 자원을 반환
	_vecGameObjects.clear();
	_hmGameObjects.clear();
	_playerIds.clear();
	_dbids.clear();
	_elos.clear();
	_points.clear();
	_winners.clear();
	_state = GameState::EndGame;
}

/*
bool PingPongGameRoom::MakeSerializedBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed, S2C_Protocol::S_P_Bullet& outPkt) {
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
		return false;
		break;
	}

	if (bulletRef == nullptr)
		return false;

	bulletRef->SetObjectId(GenerateUniqueGameObjectId());
	bulletRef->SetVector(px, pz, sx, sz, speed);
	bulletRef->UpdateTick(::GetTickCount64());
	RegisterGameObject(bulletRef);

	//2. 해당 Bullet의 생성을 Bullet의 정보를 담아 직렬화
	S2C_Protocol::UnityGameObject* bullet_ptr = outPkt.mutable_bullet();
	bullet_ptr->set_objectid(bulletRef->GetObjectId());
	bullet_ptr->set_objecttype(bulletRef->GetObjectTypeInteger());
	S2C_Protocol::XYZ* pos_ptr = bullet_ptr->mutable_position();
	pos_ptr->set_x(px);
	pos_ptr->set_y(0.2f);
	pos_ptr->set_z(pz);

	S2C_Protocol::XYZ* moveDir_ptr = outPkt.mutable_movedir();
	moveDir_ptr->set_x(sx);
	moveDir_ptr->set_z(sz);

	outPkt.set_speed(speed);
	outPkt.set_lastcollider(-1);

	return true;
}

void PingPongGameRoom::MakeBullet(int32_t bulletType, float px, float pz, float sx, float sz, float speed) {
	S2C_Protocol::S_P_Bullet pkt;
	if (!MakeSerializedBullet(bulletType, px, pz, sx, sz, speed, pkt))
		return;
	
	//TODO : 유효한 패킷인지에 대한 검사 필요
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void PingPongGameRoom::MakeBullets(initializer_list<S2C_Protocol::S_P_Bullet> serializedBullets) {
	S2C_Protocol::S_P_Bullets pkt;
	pkt.mutable_bullets()->Reserve(serializedBullets.size());

	for (const auto& bullet : serializedBullets) {
		pkt.add_bullets()->CopyFrom(bullet);
	}

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}
*/

void PingPongGameRoom::MakeBulletsFromPatternMap(const S2C_Protocol::S_P_Bullets& serializedBullets) {
	S2C_Protocol::S_P_Bullets copyPkt = serializedBullets;
	int size = copyPkt.bullets_size();
	for (int i = 0; i < size; i++) {
		S2C_Protocol::S_P_Bullet* pBullet = copyPkt.mutable_bullets(i);
		SpawnAndInitializeBullet(pBullet);
	}

	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(copyPkt);
	BroadCast(sendBuffer);
}

bool PingPongGameRoom::SpawnAndInitializeBullet(S2C_Protocol::S_P_Bullet* pSerializedBullet) {
	shared_ptr<PingPongGameBullet> bulletRef = nullptr;
	
	GameObjectType bulletType = IntToGameObjectType(pSerializedBullet->bullet().objecttype());
	float px = pSerializedBullet->bullet().position().x();
	float pz = pSerializedBullet->bullet().position().z();
	float sx = pSerializedBullet->movedir().x();
	float sz = pSerializedBullet->movedir().z();
	float speed = pSerializedBullet->speed();

	switch (bulletType) {
	case (GameObjectType::PingPongGameBulletRed): {
		shared_ptr<PingPongGameBulletRed> redBullet = PingPongGameBulletRed::NewTestGameBullet(px, 0.2f, pz);
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(redBullet);
		break;
	}
	case (GameObjectType::PingPongGameBulletBlue): {
		shared_ptr<PingPongGameBulletBlue> blueBullet = PingPongGameBulletBlue::NewTestGameBullet(px, 0.2f, pz);
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(blueBullet);
		break;
	}
	case (GameObjectType::PingPongGameBulletPupple): {
		shared_ptr<PingPongGameBulletPupple> puppleBullet = PingPongGameBulletPupple::NewTestGameBullet(px, 0.2f, pz);
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(puppleBullet);
		break;
	}
	default:
		return false;
		break;
	}

	if (bulletRef == nullptr)
		return false;

	int32_t newObjectId = GenerateUniqueGameObjectId();
	pSerializedBullet->mutable_bullet()->set_objectid(newObjectId);
	bulletRef->SetObjectId(newObjectId);
	bulletRef->SetVector(px, pz, sx, sz, speed);
	bulletRef->UpdateTick(::GetTickCount64());
	RegisterGameObject(bulletRef);
	return true;
}

void PingPongGameRoom::Handle_CollisionBar(float px, float pz, float speed, int32_t objectId, int32_t playerIdx) {
	shared_ptr<PingPongGameBullet> bulletRef;

	auto it = _hmGameObjects.find(objectId);
	if (it != _hmGameObjects.end())
		bulletRef = dynamic_pointer_cast<PingPongGameBullet>(it->second);

	if (bulletRef == nullptr)
		return;
	
	if (IsVaildCollision(bulletRef, px, pz, speed, playerIdx)) {
		//TODO : 유효한 경우, lastCollider수정. speedvector수정, position수정, 패킷 전송.
		bulletRef->_posX = px;
		bulletRef->_posZ = pz;
		if (playerIdx < 2)
			bulletRef->_moveDirX = -bulletRef->_moveDirX;
		else
			bulletRef->_moveDirZ = -bulletRef->_moveDirZ;
		bulletRef->_lastColider = playerIdx;
		bulletRef->UpdateTick(::GetTickCount64());

		S2C_Protocol::S_P_Bullet pkt;
		S2C_Protocol::UnityGameObject* pBullet = pkt.mutable_bullet();
		pBullet->set_objectid(bulletRef->GetObjectId());
		pBullet->set_objecttype(bulletRef->GetObjectTypeInteger());
		pBullet->mutable_position()->set_x(bulletRef->_posX);
		pBullet->mutable_position()->set_z(bulletRef->_posZ);
		pkt.mutable_movedir()->set_x(bulletRef->_moveDirX);
		pkt.mutable_movedir()->set_z(bulletRef->_moveDirZ);
		pkt.set_speed(bulletRef->_speed);
		pkt.set_lastcollider(bulletRef->_lastColider);

		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		BroadCast(sendBuffer);
	}
}

bool PingPongGameRoom::IsVaildCollision(shared_ptr<PingPongGameBullet> bulletRef, float px, float pz, float speed, int32_t playerIdx) {
	if (bulletRef->_lastColider == playerIdx)
		return false;

	cout << bulletRef->GetObjectId() << endl;

	uint64_t now = ::GetTickCount64();
	if (bulletRef->_updatedTick > now)
		return false;

	uint64_t deltaT = now - bulletRef->_updatedTick;
	float deltaX = (deltaT * bulletRef->_moveDirX * speed) / 1000;
	float deltaZ = (deltaT * bulletRef->_moveDirZ * speed) / 1000;
	
	//원래 deltaPos는 차합이 아니라 제곱합을 사용해야 하지만 성능을 위해 최대 루트2배의 오차를 감안하고 이대로 사용.
	float deltaPos = abs(bulletRef->_posX + deltaX - px) + abs(bulletRef->_posZ + deltaZ - pz);
	float toler = pPingPongManager->GetToleranceRate();

	if (deltaPos > toler * bulletRef->_speed)
		return false;
	
	return true;
}

void PingPongGameRoom::Handle_CollisionGoalLine(int32_t playerIdx, int32_t point) {
	_points[playerIdx] = _points[playerIdx] - point;
}

void PingPongGameRoom::Handle_Response_KeepAlive(int32_t playerIdx) {
	//TODO: 받은 tick과 지금 tick을 비교. 일치할 경우, 해당 tick에 대해서 점수를 받았는지 확인.
		//받지 않았을 경우, 점수 추가. 받았을 경우, 패킷 조작범일 가능성
	//해당 PlayerTick이 일정 기간동안 update되지 않는 경우, 유효하지 않은 연결로 간주해서 내보내는 기능 추가도 고려하는중.
		//일단은 시험용으로 받는족족 점수를 추가해주기만 한다.
	_points[playerIdx] = _points[playerIdx] + 10;
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

void PingPongGameRoom::RenewScoreBoard() {
	if (_updateCount % 10 != 0)
		return;

	S2C_Protocol::S_P_RenewScores pkt;
	for (auto& point : _points)	pkt.add_scores(point);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void PingPongGameRoom::BroadCastKeepAlive() {
	if (_updateCount % 30 != 0)
		return;

	_keepAliveTick = ::GetTickCount64();
	_keepAlivePkt.set_tick(_keepAliveTick);

	for (auto& playerSessionWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerSessionRef = playerSessionWRef.lock();
		if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
			continue;

		S2C_Protocol::S_Encrypted pkt = S2CPacketMaker::MakeSEncrypted(_keepAlivePkt, PKT_S_P_KEEP_ALIVE, playerSessionRef->GetAESKey());
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
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

	_updateCount++;
	RenewScoreBoard();
	RequestPlayerBarPosition();
	BroadCastKeepAlive();
}

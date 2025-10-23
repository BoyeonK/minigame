#include "pch.h"
#include "S2CPacketHandler.h"
#include "PlayerSession.h"
#include "S2CPacketMaker.h"
#include "GameRoom.h"
#include "PingPongGameRoom.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];
PlaintextHandlerFunc PlaintextHandler[UINT16_MAX];

bool Handle_Invalid(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
	cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
	return false;
}

bool Handle_C_Encrypted(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Encrypted& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	//TODO : Session�� ����� AESKey�� ���ؼ� ���� Protobuf�� ������ ��, Handler�Լ��� ���۽�Ų��.
	const string& iv_str = pkt.iv();
	const string& ciphertext_str = pkt.ciphertext();
	const string& tag_str = pkt.tag();
	int32_t msgId = pkt.msgid();

	vector<unsigned char> aesKey = playerSessionRef->GetAESKey();

	vector<unsigned char> aad(sizeof(msgId));
	memcpy(aad.data(), &msgId, sizeof(msgId));

	auto ctx_deleter = [](EVP_CIPHER_CTX* ctx) { EVP_CIPHER_CTX_free(ctx); };
	unique_ptr<EVP_CIPHER_CTX, decltype(ctx_deleter)> ctx(EVP_CIPHER_CTX_new(), ctx_deleter);

	if (!ctx) {
		std::cerr << "Error: EVP_CIPHER_CTX_new failed." << std::endl;
		return false;
	}

	if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
		std::cerr << "Error: EVP_DecryptInit_ex (cipher) failed." << std::endl;
		return false;
	}

	if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, iv_str.length(), NULL) != 1) {
		std::cerr << "Error: EVP_CIPHER_CTX_ctrl (IV length) failed." << std::endl;
		return false;
	}

	if (EVP_DecryptInit_ex(ctx.get(), NULL, NULL, aesKey.data(), (const unsigned char*)iv_str.c_str()) != 1) {
		std::cerr << "Error: EVP_DecryptInit_ex (key, IV) failed." << std::endl;
		return false;
	}

	int len = 0;
	if (EVP_DecryptUpdate(ctx.get(), NULL, &len, aad.data(), aad.size()) != 1) {
		std::cerr << "Error: EVP_DecryptUpdate (AAD) failed." << std::endl;
		return false;
	}

	std::vector<unsigned char> plaintext(ciphertext_str.length());
	if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, (const unsigned char*)ciphertext_str.c_str(), ciphertext_str.length()) != 1) {
		std::cerr << "Error: EVP_DecryptUpdate (ciphertext) failed." << std::endl;
		return false;
	}
	int plaintext_len = len;

	if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, tag_str.length(), (void*)tag_str.c_str()) != 1) {
		std::cerr << "Error: EVP_CIPHER_CTX_ctrl (set tag) failed." << std::endl;
		return false;
	}

	if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
		std::cerr << "Error: EVP_DecryptFinal_ex failed. Tag verification failed." << std::endl;
		// ���� ���д� �ſ� �߿��� ���� �̺�Ʈ�Դϴ�. �α׸� ����� ���� �����ϴ�.
		return false;
	}
	plaintext_len += len;
	plaintext.resize(plaintext_len);
	
	cout << "plaintext ����" << endl;

	if (msgId > sessionRef->GetSecureLevel()) {
		cout << "���� ������ ���� �ʴ� ��Ŷ. " << endl;
		return false;
	}

	return PlaintextHandler[msgId](sessionRef, plaintext);
}

bool Handle_C_Welcome(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Welcome& recvPkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	cout << "C_Welcome ��Ŷ�� �޾Ҵ�." << endl;

	string encryptedStr = recvPkt.aeskey();
	vector<unsigned char> encryptedKey(encryptedStr.begin(), encryptedStr.end());
	vector<unsigned char> AESKey = CryptoManager::Decrypt(playerSessionRef->GetRSAKey(), encryptedKey);

	if (AESKey.empty()) {
		//TODO : �������� AESKey�� Ȯ������ ������ ��� ����ó��
		cout << "AES Key ��ȣȭ ����!" << endl;
		playerSessionRef->Disconnect();
		return false;
	}

	playerSessionRef->SetAESKey(move(AESKey));

	S2C_Protocol::S_WelcomeResponse sendPkt = S2CPacketMaker::MakeSWelcomeResponse(recvPkt.message(), true);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(sendPkt, playerSessionRef->GetAESKey());

	/*
	S2C_Protocol::S_WelcomeResponse baseSendPkt = S2CPacketMaker::MakeSWelcomeResponse(recvPkt.message(), true);
	S2C_Protocol::S_Encrypted sendPkt = S2CPacketMaker::MakeSEncrypted(baseSendPkt, PKT_S_WELCOMERESPONSE, playerSessionRef->GetAESKey());
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(sendPkt);
	*/

	if (sendBuffer == nullptr) {
		return false;
	}
	playerSessionRef->Send(sendBuffer);
	playerSessionRef->SetSecureLevel(8);

	cout << "S_WelcomeResponse ����" << endl;

	return true;
}

bool Handle_C_Login(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Login& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;

	//playerSessionRef->SetPlayerId(pkt.id());
	return DBManager->S2D_Login(sessionRef, pkt.id(), pkt.password());
}

bool Handle_C_CreateAccount(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_CreateAccount& pkt) {
	return DBManager->S2D_CreateAccount(sessionRef, pkt.id(), pkt.password());
}

bool Handle_C_Logout(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Logout& pkt) {
	PlayerSession* playerSessionRef = static_cast<PlayerSession*>(sessionRef.get());
	bool isSucceed = false;
	if (pkt.dbid() == playerSessionRef->GetDbid()) {
		playerSessionRef->SetDbid(0);
		playerSessionRef->SetSecureLevel(8);
		S2C_Protocol::S_Logout pkt = S2CPacketMaker::MakeSLogout(true);
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBufferRef);
	}
	else {
		//�� ���ǹ��� else�� ���� �� ��Ȳ�� �ſ� �̻��ϴ�. 
		//Ư��, ���� dbid�� 0�� �ƴ� ����� ������ ���� �ǽɵǴ� ��Ȳ.
		if (pkt.dbid() != 0) {
			//�α����� ������ '�ٸ� ����'�� dbid�� ����Ϸ��� �ϰ� �ִ�.
		}
		playerSessionRef->SetDbid(0);
		playerSessionRef->SetSecureLevel(8);

		S2C_Protocol::S_Logout responsePkt = S2CPacketMaker::MakeSLogout(false);
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(responsePkt);
		playerSessionRef->Send(sendBufferRef);
	}
	return isSucceed;
}

bool Handle_C_MatchmakeRequestInternal(shared_ptr<PlayerSession> playerSessionRef, bool isSucceed, int gameId, const string& err) {
	S2C_Protocol::S_MatchmakeRequest responsePkt = S2CPacketMaker::MakeSMatchmakeRequest(isSucceed, gameId, err);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(responsePkt);
	playerSessionRef->Send(sendBuffer);
	return isSucceed;
}

bool Handle_C_MatchmakeRequest(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_MatchmakeRequest& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);

	auto it = GGameManagers.find(pkt.gameid());
	if (it == GGameManagers.end()) {
		return Handle_C_MatchmakeRequestInternal(playerSessionRef, false, pkt.gameid(), "�ش� Manager�� �غ���� ����");
	}
		
	int32_t elo = playerSessionRef->GetElo(pkt.gameid());
	if (elo == 0) {
		return Handle_C_MatchmakeRequestInternal(playerSessionRef, false, pkt.gameid(), "elo������ �ҷ����µ� �����߽��ϴ�.");
	}
		
	GameType expected = GameType::None;
	GameType desired = IntToGameType(pkt.gameid());
	if (desired == GameType::Undefined) {
		return Handle_C_MatchmakeRequestInternal(playerSessionRef, false, pkt.gameid(), "���� ��Ŷ ����������");
	}

	WatingPlayerData pd;
	if (!playerSessionRef->TryChangeMatchingState(expected, desired)) {
		return Handle_C_MatchmakeRequestInternal(playerSessionRef, false, pkt.gameid(), "����ȭ ���� �߻�. �������� �����մϴ�.");
	}

	pd.elo = elo;
	pd.playerSessionWRef = playerSessionRef;
	pd.queuedTick = ::GetTickCount64();
	it->second->Push(move(pd));
#ifdef _DEBUG
	cout << "�ӽ� ��⿭�� ����." << endl;
#endif 
	return Handle_C_MatchmakeRequestInternal(playerSessionRef, true, pkt.gameid(), "");
}

bool Handle_C_MatchmakeCancelInternal(shared_ptr<PlayerSession> playerSessionRef, bool isSucceed, int gameId, const string& err) {
	S2C_Protocol::S_MatchmakeCancel responsePkt = S2CPacketMaker::MakeSMatchmakeCancel(isSucceed, gameId, err);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(responsePkt);
	playerSessionRef->Send(sendBuffer);
	return isSucceed;
}

bool Handle_C_MatchmakeCancel(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_MatchmakeCancel& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);

	GameType expected = IntToGameType(pkt.gameid());
	GameType desired = GameType::None;
	if (expected == GameType::Undefined) {
		return Handle_C_MatchmakeCancelInternal(playerSessionRef, false, pkt.gameid(), "��ȿ�� gameId�� �ƴ�");
	}
		
	//���� gameId�� ��Ī�� ��� �õ�.
	if (!playerSessionRef->TryChangeMatchingState(expected, desired)) {
		//��ġ �Ϸᰡ ����� ���� ����ȭ ������ ���� �ɰ��� ��.
		return Handle_C_MatchmakeCancelInternal(playerSessionRef, false, pkt.gameid(), "��ġ �Ϸ�� ť�� �ְų�, ����ȭ ����");
	}

	return Handle_C_MatchmakeCancelInternal(playerSessionRef, true, pkt.gameid(), "");
}

bool Handle_C_MatchmakeKeepAlive(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_MatchmakeKeepAlive& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;

	if (playerSessionRef->GetMatchingState() != IntToGameType(pkt.gameid())) {
		//��� ��⿭���� ��� ��Ȳ���� �뺸�ϰ�, ��⿭ ���¸� None���� �ٲ�.
		//Ŭ���̾�Ʈ�� �Ͽ��� ���� ���·� ���ư� �� �ֵ��� ���
		S2C_Protocol::S_ExcludedFromMatch pkt = S2CPacketMaker::MakeSExcludedFromMatch(false);
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBufferRef);
		playerSessionRef->SetMatchingState(GameType::None);
		return false;
	}

	playerSessionRef->SetLastKeepAliveTick(pkt.senttimetick());
	return true;
}

bool Handle_C_GameSceneLoadingProgress(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_GameSceneLoadingProgress& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	//���� : GameRoom�� nullptr��
	shared_ptr<GameRoom> gameRoom = playerSessionRef->GetJoinedRoom();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;
	if (gameRoom == nullptr)
		return false;
	gameRoom->PostEvent(&GameRoom::UpdateProgressBar, playerSessionRef->GetRoomIdx(), pkt.persentage());
	return true;
}

bool Handle_C_RequestGameState(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_RequestGameState& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	shared_ptr<GameRoom> roomRef = playerSessionRef->GetJoinedRoom();

	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;
	if (roomRef == nullptr)
		return false;

	roomRef->PostEvent(&GameRoom::SendGameState, playerSessionRef->GetRoomIdx());
	return true;
}

bool Handle_C_P_ResponsePlayerBarPosition(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_P_ResponsePlayerBarPosition& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	shared_ptr<PingPongGameRoom> roomRef = dynamic_pointer_cast<PingPongGameRoom>(playerSessionRef->GetJoinedRoom());
	
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;
	if (roomRef == nullptr)
		return false;

	roomRef->PostEvent(&PingPongGameRoom::ResponsePlayerBarPosition, playerSessionRef->GetRoomIdx(), pkt.position().x(), pkt.position().z());
	return true;
}

bool Handle_C_P_CollisionBar(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_P_CollisionBar& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	shared_ptr<PingPongGameRoom> roomRef = dynamic_pointer_cast<PingPongGameRoom>(playerSessionRef->GetJoinedRoom());
	
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;
	if (roomRef == nullptr)
		return false;

	roomRef->PostEvent(&PingPongGameRoom::Handle_CollisionBar, pkt.bullet().position().x(), pkt.bullet().position().z(), pkt.speed(), pkt.bullet().objectid(), playerSessionRef->GetRoomIdx());
	return true;
}

bool Handle_C_P_CollisionGoalLine(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_P_CollisionGoalLine& pkt) {
	shared_ptr<PlayerSession> playerSessionRef = dynamic_pointer_cast<PlayerSession>(sessionRef);
	shared_ptr<PingPongGameRoom> roomRef = dynamic_pointer_cast<PingPongGameRoom>(playerSessionRef->GetJoinedRoom());
	
	if (roomRef == nullptr)
		return false;
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return false;
	if (playerSessionRef->GetRoomIdx() >= 4 || playerSessionRef->GetRoomIdx() < 0)
		return false;

	roomRef->PostEvent(&PingPongGameRoom::Handle_CollisionGoalLine, pkt.point(), playerSessionRef->GetRoomIdx());
	return true;
}
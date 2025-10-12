#pragma once
#include "S2C_Protocol.pb.h"
#include "ServerGlobal.h"

using PacketHandlerFunc = function<bool(shared_ptr<PBSession>, unsigned char*, int32_t)>;
using PlaintextHandlerFunc = function<bool(shared_ptr<PBSession>, vector<unsigned char>&)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];
extern PlaintextHandlerFunc PlaintextHandler[UINT16_MAX];

//name convention : �������� ������(Ŭ�� �޴�) S_
//					Ŭ�󿡼� ������(������ �޴�) C_
enum : uint16_t {
	PKT_S_WELCOME = 0,
	PKT_C_WELCOME = 1,
	PKT_S_WELCOME_RESPONSE = 2,
	PKT_S_ENCRYPTED = 3,
	PKT_C_ENCRYPTED = 4,
	PKT_C_LOGIN = 5,
	PKT_S_LOGIN = 6,
	PKT_C_CREATE_ACCOUNT = 7,
	PKT_S_CREATE_ACCOUNT = 8,
	PKT_C_LOGOUT = 9,
	PKT_S_LOGOUT = 10,
	PKT_C_MATCHMAKEREQUEST = 11,
	PKT_S_MATCHMAKEREQUEST = 12,
	PKT_C_MATCHMAKECANCEL = 13,
	PKT_S_MATCHMAKECANCEL = 14,
	PKT_S_MATCHMAKEKEEPALIVE = 15,
	PKT_C_MATCHMAKEKEEPALIVE = 16,
	PKT_S_REDOMATCHMAKE = 17,
	PKT_S_EXCLUDEDFROMMATCH = 18,
	PKT_S_MATCHMAKECOMPLETED = 19,
	PKT_C_GAMESCENELOADINGPROGRESS = 20,
	PKT_S_GAME_STARTED = 21,
	PKT_C_REQUEST_GAME_STATE = 22,
	PKT_S_DELTA_GAME_OBJECT_SOFT = 23,
	PKT_S_DELTA_GAME_OBJECT_HARD = 24,
	PKT_S_SPAWN_GAME_OBJECT = 25,
	PKT_S_DESPAWN_GAME_OBJECT = 26,
	PKT_S_END_GAME = 27,

	PKT_S_TESTGAME_STATE = 100,
	PKT_S_TESTGAME_RESULT = 101,

	PKT_S_P_STATE = 200,
	PKT_S_P_RESULT = 201,
	PKT_S_P_READY_FOR_START = 202,
	PKT_S_P_REQUEST_PLAYER_BAR_POSITION = 203,
	PKT_C_P_RESPONSE_PLAYER_BAR_POSITION = 204,
	PKT_S_P_CHANGE_PLAYER_BAR_POSITION = 205,
	PKT_S_P_BULLET = 206,
	PKT_C_P_COLLISION_BAR = 207,
	PKT_C_P_COLLISION_GOAL_LINE = 208,

	PKT_S_DANMAKU_STATE = 300,
	PKT_S_DANMAKU_RESTULT = 301,
};

bool Handle_Invalid(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len);
bool Handle_C_Encrypted(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Encrypted& pkt);
bool Handle_C_Welcome(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Welcome& pkt);
bool Handle_C_Login(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Login& pkt);
bool Handle_C_CreateAccount(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_CreateAccount& pkt);
bool Handle_C_Logout(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_Logout& pkt);
bool Handle_C_MatchmakeRequest(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_MatchmakeRequest& pkt);
bool Handle_C_MatchmakeCancel(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_MatchmakeCancel& pkt);
bool Handle_C_MatchmakeKeepAlive(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_MatchmakeKeepAlive& pkt);
bool Handle_C_GameSceneLoadingProgress(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_GameSceneLoadingProgress& pkt);
bool Handle_C_RequestGameState(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_RequestGameState& pkt);

	//PingPong
bool Handle_C_P_ResponsePlayerBarPosition(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_P_ResponsePlayerBarPosition& pkt);
bool Handle_C_P_CollisionBar(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_P_CollisionBar& pkt);
bool Handle_C_P_CollisionGoalLine(shared_ptr<PBSession> sessionRef, S2C_Protocol::C_P_CollisionGoalLine& pkt);

class S2CPacketHandler {
public:
	static void Init() {
		for (int32_t i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_Invalid;
		
		//Ŭ���̾�Ʈ�κ��� ���� ���̳ʸ����� ����� ������ ������ �κ��� �˸��� protobufŸ������ ĳ�����ϰ�, �˸´� �ڵ鷯 �Լ��� ȣ��.
		GPacketHandler[PKT_C_ENCRYPTED] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Encrypted>(Handle_C_Encrypted, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Welcome>(Handle_C_Welcome, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Login>(Handle_C_Login, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_CREATE_ACCOUNT] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_CreateAccount>(Handle_C_CreateAccount, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_LOGOUT] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_Logout>(Handle_C_Logout, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_MATCHMAKEREQUEST] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_MatchmakeRequest>(Handle_C_MatchmakeRequest, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_MATCHMAKECANCEL] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_MatchmakeCancel>(Handle_C_MatchmakeCancel, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_MATCHMAKEKEEPALIVE] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_MatchmakeKeepAlive>(Handle_C_MatchmakeKeepAlive, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_GAMESCENELOADINGPROGRESS] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_GameSceneLoadingProgress>(Handle_C_GameSceneLoadingProgress, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_REQUEST_GAME_STATE] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_RequestGameState>(Handle_C_RequestGameState, sessionRef, buffer, len); };

			//PingPong
		GPacketHandler[PKT_C_P_RESPONSE_PLAYER_BAR_POSITION] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_P_ResponsePlayerBarPosition>(Handle_C_P_ResponsePlayerBarPosition, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_P_COLLISION_BAR] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_P_CollisionBar>(Handle_C_P_CollisionBar, sessionRef, buffer, len); };
		GPacketHandler[PKT_C_P_COLLISION_GOAL_LINE] = [](shared_ptr<PBSession>sessionRef, unsigned char* buffer, int32_t len) { return HandlePacket<S2C_Protocol::C_P_CollisionGoalLine>(Handle_C_P_CollisionGoalLine, sessionRef, buffer, len); };

		//C_Encrypted�� ��ȣȭ�Ͽ� ���� ���̳ʸ��� �˸��� protobufŸ������ ĳ�����ϰ�, �˸��� �ڵ鷯 �Լ��� ȣ��.
		//��ȣȭ ���� ���� ��Ŷ�� ���ؼ� PlaintextHandler�� ������ ä�� �ʿ�� ������, �ؼ� ���ܰ� �����ϱ�.
		PlaintextHandler[PKT_C_WELCOME] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Welcome>(Handle_C_Welcome, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_LOGIN] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Login>(Handle_C_Login, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_CREATE_ACCOUNT] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_CreateAccount>(Handle_C_CreateAccount, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_LOGOUT] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_Logout>(Handle_C_Logout, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_MATCHMAKEREQUEST] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_MatchmakeRequest>(Handle_C_MatchmakeRequest, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_MATCHMAKECANCEL] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_MatchmakeCancel>(Handle_C_MatchmakeCancel, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_MATCHMAKEKEEPALIVE] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_MatchmakeKeepAlive>(Handle_C_MatchmakeKeepAlive, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_GAMESCENELOADINGPROGRESS] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_GameSceneLoadingProgress>(Handle_C_GameSceneLoadingProgress, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_REQUEST_GAME_STATE] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_RequestGameState>(Handle_C_RequestGameState, sessionRef, plaintext); };

			//PingPong
		PlaintextHandler[PKT_C_P_RESPONSE_PLAYER_BAR_POSITION] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_P_ResponsePlayerBarPosition>(Handle_C_P_ResponsePlayerBarPosition, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_P_COLLISION_BAR] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_P_CollisionBar>(Handle_C_P_CollisionBar, sessionRef, plaintext); };
		PlaintextHandler[PKT_C_P_COLLISION_GOAL_LINE] = [](shared_ptr<PBSession> sessionRef, vector<unsigned char>& plaintext) { return HandlePlaintext<S2C_Protocol::C_P_CollisionGoalLine>(Handle_C_P_CollisionGoalLine, sessionRef, plaintext); };
	}

	static bool HandlePacket(shared_ptr<PBSession> sessionRef, unsigned char* buffer, int32_t len) {
		//TODO: Session�� ����� ������ pktId�� �ڵ鷯�� �����ϱ�.
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		if (header->_id > sessionRef->GetSecureLevel()) {
			cout << "���� ������ ���� �ʴ� ��Ŷ. " << endl;
			return false;
		}

		return GPacketHandler[header->_id](sessionRef, buffer, len);
	}

	//������� Protobuf�� SendBufferChunk�� �ε��ϰ� SendBuffer�� �Ҵ���� �Լ���.
#pragma region Lobby
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Encrypted& pkt) { return MakeSendBufferRef(pkt, PKT_S_ENCRYPTED); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Welcome& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOME); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_WelcomeResponse& pkt) { return MakeSendBufferRef(pkt, PKT_S_WELCOME_RESPONSE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Login& pkt) { return MakeSendBufferRef(pkt, PKT_S_LOGIN); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_CreateAccount& pkt) { return MakeSendBufferRef(pkt, PKT_S_CREATE_ACCOUNT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Logout& pkt) { return MakeSendBufferRef(pkt, PKT_S_LOGOUT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeRequest& pkt) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKEREQUEST); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeCancel& pkt) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKECANCEL); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeKeepAlive& pkt) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKEKEEPALIVE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_RedoMatchmake& pkt) { return MakeSendBufferRef(pkt, PKT_S_REDOMATCHMAKE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_ExcludedFromMatch& pkt) { return MakeSendBufferRef(pkt, PKT_S_EXCLUDEDFROMMATCH); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeCompleted& pkt) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKECOMPLETED); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_GameStarted& pkt) { return MakeSendBufferRef(pkt, PKT_S_GAME_STARTED); }

	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Welcome& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOME, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_WelcomeResponse& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_WELCOME_RESPONSE, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Login& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_LOGIN, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_CreateAccount& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_CREATE_ACCOUNT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_Logout& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_LOGOUT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeRequest& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKEREQUEST, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeCancel& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKECANCEL, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeKeepAlive& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKEKEEPALIVE, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_RedoMatchmake& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_REDOMATCHMAKE, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_ExcludedFromMatch& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_EXCLUDEDFROMMATCH, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_MatchmakeCompleted& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_MATCHMAKECOMPLETED, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_GameStarted& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_GAME_STARTED, AESKey); }
#pragma endregion

#pragma region IngameCommon
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DeltaGameObjectSoft& pkt) { return MakeSendBufferRef(pkt, PKT_S_DELTA_GAME_OBJECT_SOFT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DeltaGameObjectHard& pkt) { return MakeSendBufferRef(pkt, PKT_S_DELTA_GAME_OBJECT_HARD); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_SpawnGameObject& pkt) { return MakeSendBufferRef(pkt, PKT_S_SPAWN_GAME_OBJECT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DespawnGameObject& pkt) { return MakeSendBufferRef(pkt, PKT_S_DESPAWN_GAME_OBJECT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_EndGame& pkt) { return MakeSendBufferRef(pkt, PKT_S_END_GAME); }

	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DeltaGameObjectSoft& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_DELTA_GAME_OBJECT_SOFT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DeltaGameObjectHard& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_DELTA_GAME_OBJECT_HARD, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_SpawnGameObject& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_SPAWN_GAME_OBJECT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DespawnGameObject& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_DESPAWN_GAME_OBJECT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_EndGame& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_END_GAME, AESKey); }
#pragma endregion

#pragma region TestGame
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_TestGameState& pkt) { return MakeSendBufferRef(pkt, PKT_S_TESTGAME_STATE); }

	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_TestGameState& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_TESTGAME_STATE, AESKey); }
#pragma endregion

#pragma region PingPong
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_State& pkt) { return MakeSendBufferRef(pkt, PKT_S_P_STATE); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_Result& pkt) { return MakeSendBufferRef(pkt, PKT_S_P_RESULT); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_ReadyForStart& pkt) { return MakeSendBufferRef(pkt, PKT_S_P_READY_FOR_START); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_RequestPlayerBarPosition& pkt) { return MakeSendBufferRef(pkt, PKT_S_P_REQUEST_PLAYER_BAR_POSITION); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_ChangePlayerBarPosition& pkt) { return MakeSendBufferRef(pkt, PKT_S_P_CHANGE_PLAYER_BAR_POSITION); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_Bullet& pkt) { return MakeSendBufferRef(pkt, PKT_S_P_BULLET); }

	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_State& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_P_STATE, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_Result& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_P_RESULT, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_ReadyForStart& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_P_READY_FOR_START, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_RequestPlayerBarPosition& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_P_REQUEST_PLAYER_BAR_POSITION, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_ChangePlayerBarPosition& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_P_CHANGE_PLAYER_BAR_POSITION, AESKey); }
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_P_Bullet& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_P_BULLET, AESKey); }

#pragma endregion

#pragma region Danmaku
	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DanmakuState& pkt) { return MakeSendBufferRef(pkt, PKT_S_DANMAKU_STATE); }

	static shared_ptr<SendBuffer> MakeSendBufferRef(const S2C_Protocol::S_DanmakuState& pkt, const vector<unsigned char>& AESKey) { return MakeSendBufferRef(pkt, PKT_S_DANMAKU_STATE, AESKey); }
#pragma endregion

private:
	template<typename PBType, typename HandlerFunc>
	static bool HandlePacket(HandlerFunc func, shared_ptr<PBSession>& sessionRef, unsigned char* buffer, int32_t len) {
		PBType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;
		return func(sessionRef, pkt);
	}

	template<typename PBType, typename HandlerFunc>
	static bool HandlePlaintext(HandlerFunc func, shared_ptr<PBSession>& sessionRef, vector<unsigned char> plaintext) {
		PBType pkt;
		if (pkt.ParseFromArray(plaintext.data(), plaintext.size()) == false)
			return false;
		return func(sessionRef, pkt);
	}

	//PB�� ����ȭ�� ��Ŷ�� Local SendBufferChunk�� �ε��ϰ�
	//�ش� SendBuffer�� Return
	template<typename PBType>
	static shared_ptr<SendBuffer> MakeSendBufferRef(const PBType& pkt, uint16_t pktId) {
		uint16_t dataSize = static_cast<uint16_t>(pkt.ByteSizeLong());
		uint16_t packetSize = dataSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBufferRef = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBufferRef->Buffer());
		header->_size = packetSize;
		header->_id = pktId;
		pkt.SerializeToArray(&header[1]/*(++header)*/, dataSize);
		sendBufferRef->Close(packetSize);

		return sendBufferRef;
	}

	//PB�� ����ȭ�� ��Ŷ�� AESŰ�� �̿��Ͽ� ��ȣȭ, SendBufferChunk�� �ε��ϰ� �ش� SendBuffer�� Return
	template<typename PBType>
	static shared_ptr<SendBuffer> MakeSendBufferRef(const PBType& pkt, uint16_t pktId, const vector<unsigned char>& AESKey) {
		//C#�̾��ٸ� pkt�� where IMessage���� Ȯ���ؼ� SerializeAsString() �޼��带 ������ �ִ��� 
		//Ȯ���ϰ� �����ϰ� �۵������� �ٵ�..
		string serializedStr = pkt.SerializeAsString();
		vector<unsigned char> plaintext(serializedStr.begin(), serializedStr.end());
		vector<unsigned char> iv, ciphertext, tag;
		if (!(GCryptoManager->Encrypt(AESKey, plaintext, (int32_t)pktId, iv, ciphertext, tag))) {
			cout << "��ȣȭ ����" << endl;
			return nullptr;
		}
		S2C_Protocol::S_Encrypted sendPkt;
		sendPkt.set_iv(iv.data(), iv.size());
		sendPkt.set_ciphertext(ciphertext.data(), ciphertext.size());
		sendPkt.set_tag(tag.data(), tag.size());
		sendPkt.set_msgid(pktId);

		uint16_t dataSize = static_cast<uint16_t>(sendPkt.ByteSizeLong());
		uint16_t packetSize = dataSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBufferRef = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBufferRef->Buffer());
		header->_size = packetSize;
		header->_id = PKT_S_ENCRYPTED;
		sendPkt.SerializeToArray(&header[1]/*(++header)*/, dataSize);
		sendBufferRef->Close(packetSize);

		return sendBufferRef;
	}
};
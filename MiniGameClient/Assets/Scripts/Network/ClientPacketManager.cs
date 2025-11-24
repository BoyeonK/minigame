using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections.Generic;

class PacketManager {
	#region Singleton
	static PacketManager _instance = new PacketManager();
	public static PacketManager Instance { get { return _instance; } }
	#endregion

	PacketManager()	{
		Register();
	}

	Dictionary<ushort, Action<PacketSession, ArraySegment<byte>, ushort>> _onRecv = new Dictionary<ushort, Action<PacketSession, ArraySegment<byte>, ushort>>();
	Dictionary<ushort, Action<PacketSession, IMessage>> _handler = new Dictionary<ushort, Action<PacketSession, IMessage>>();
	Dictionary<ushort, Func<IMessage>> _msgFactories = new Dictionary<ushort, Func<IMessage>>();
	
	//PacketManager 초기화 시, 각 패킷을 다룰 델리게이트들을 초기화해준다.
	//_onRecv는 서버로부터 받은 바이너리에서 추출한 패킷 헤더의 msgId를 토대로
	// 바이너리를 헤더와 protobuf부분으로 나누고, protobuf부분을 알맞게 캐스팅한다.
	//_handler는 캐스팅된 protobuf내용에 따라 호출할 handler함수의 집합이다.
	//_msgFactories는 SEncrypted를 통해 복호화된 바이너리를 알맞은 protobuf로 parse하기 위한
	//factory함수이다.
	public void Register() {
		_onRecv.Add((ushort)MsgId.SEncrypted, UnpackPacket<S_Encrypted>);
		_handler.Add((ushort)MsgId.SEncrypted, PacketHandler.S_EncryptedHandler);

		_onRecv.Add((ushort)MsgId.SWelcome, UnpackPacket<S_Welcome>);
		_handler.Add((ushort)MsgId.SWelcome, PacketHandler.S_WelcomeHandler);
		_msgFactories.Add((ushort)MsgId.SWelcome, () => new S_Welcome());

		_onRecv.Add((ushort)MsgId.SWelcomeResponse, UnpackPacket<S_WelcomeResponse>);
		_handler.Add((ushort)MsgId.SWelcomeResponse, PacketHandler.S_WelcomeResponseHandler);
		_msgFactories.Add((ushort)MsgId.SWelcomeResponse, () => new S_WelcomeResponse());

		_onRecv.Add((ushort)MsgId.SLogin, UnpackPacket<S_Login>);
		_handler.Add((ushort)MsgId.SLogin, PacketHandler.S_LoginHandler);
		_msgFactories.Add((ushort)MsgId.SLogin, () => new S_Login());

		_onRecv.Add((ushort)MsgId.SCreateAccount, UnpackPacket<S_CreateAccount>);
		_handler.Add((ushort)MsgId.SCreateAccount, PacketHandler.S_CreateAccountHandler);
		_msgFactories.Add((ushort)MsgId.SCreateAccount, () => new S_CreateAccount());

        _onRecv.Add((ushort)MsgId.SLogout, UnpackPacket<S_Logout>);
        _handler.Add((ushort)MsgId.SLogout, PacketHandler.S_LogoutHandler);
        _msgFactories.Add((ushort)MsgId.SLogout, () => new S_Logout());

        _onRecv.Add((ushort)MsgId.SMatchmakeRequest, UnpackPacket<S_MatchmakeRequest>);
        _handler.Add((ushort)MsgId.SMatchmakeRequest, PacketHandler.S_MatchmakeRequestHandler);
        _msgFactories.Add((ushort)MsgId.SMatchmakeRequest, () => new S_MatchmakeRequest());

        _onRecv.Add((ushort)MsgId.SMatchmakeCancel, UnpackPacket<S_MatchmakeCancel>);
        _handler.Add((ushort)MsgId.SMatchmakeCancel, PacketHandler.S_MatchmakeCancelHandler);
        _msgFactories.Add((ushort)MsgId.SMatchmakeCancel, () => new S_MatchmakeCancel());

        _onRecv.Add((ushort)MsgId.SMatchmakeKeepAlive, UnpackPacket<S_MatchmakeKeepAlive>);
        _handler.Add((ushort)MsgId.SMatchmakeKeepAlive, PacketHandler.S_MatchmakeKeepAliveHandler);
        _msgFactories.Add((ushort)MsgId.SMatchmakeKeepAlive, () => new S_MatchmakeKeepAlive());

        _onRecv.Add((ushort)MsgId.SExcludedFromMatch, UnpackPacket<S_ExcludedFromMatch>);
        _handler.Add((ushort)MsgId.SExcludedFromMatch, PacketHandler.S_ExcludedFromMatchHandler);
        _msgFactories.Add((ushort)MsgId.SExcludedFromMatch, () => new S_ExcludedFromMatch());

        _onRecv.Add((ushort)MsgId.SMatchmakeCompleted, UnpackPacket<S_MatchmakeCompleted>);
        _handler.Add((ushort)MsgId.SMatchmakeCompleted, PacketHandler.S_MatchmakeCompletedHandler);
        _msgFactories.Add((ushort)MsgId.SMatchmakeCompleted, () => new S_MatchmakeCompleted());

        _onRecv.Add((ushort)MsgId.SGameStarted, UnpackPacket<S_GameStarted>);
        _handler.Add((ushort)MsgId.SGameStarted, PacketHandler.S_GameStartedHandler);
        _msgFactories.Add((ushort)MsgId.SGameStarted, () => new S_GameStarted());

        _onRecv.Add((ushort)MsgId.STestgameState, UnpackPacket<S_TestGameState>);
        _handler.Add((ushort)MsgId.STestgameState, PacketHandler.S_TestGameStateHandler);
        _msgFactories.Add((ushort)MsgId.STestgameState, () => new S_TestGameState());

        _onRecv.Add((ushort)MsgId.SSpawnGameObject, UnpackPacket<S_SpawnGameObject>);
        _handler.Add((ushort)MsgId.SSpawnGameObject, PacketHandler.S_SpawnGameObjectHandler);
        _msgFactories.Add((ushort)MsgId.SSpawnGameObject, () => new S_SpawnGameObject());

        _onRecv.Add((ushort)MsgId.SEndGame, UnpackPacket<S_EndGame>);
        _handler.Add((ushort)MsgId.SEndGame, PacketHandler.S_EndGameHandler);
        _msgFactories.Add((ushort)MsgId.SEndGame, () => new S_EndGame());

        _onRecv.Add((ushort)MsgId.SPState, UnpackPacket<S_P_State>);
        _handler.Add((ushort)MsgId.SPState, PacketHandler.S_P_StateHandler);
        _msgFactories.Add((ushort)MsgId.SPState, () => new S_P_State());

        _onRecv.Add((ushort)MsgId.SPRequestPlayerBarPosition, UnpackPacket<S_P_RequestPlayerBarPosition>);
        _handler.Add((ushort)MsgId.SPRequestPlayerBarPosition, PacketHandler.S_P_RequestPlayerBarPositionHandler);
        _msgFactories.Add((ushort)MsgId.SPRequestPlayerBarPosition, () => new S_P_RequestPlayerBarPosition());

        _onRecv.Add((ushort)MsgId.SPBullet, UnpackPacket<S_P_Bullet>);
        _handler.Add((ushort)MsgId.SPBullet, PacketHandler.S_P_BulletHandler);
        _msgFactories.Add((ushort)MsgId.SPBullet, () => new S_P_Bullet());

        _onRecv.Add((ushort)MsgId.SPBullets, UnpackPacket<S_P_Bullets>);
        _handler.Add((ushort)MsgId.SPBullets, PacketHandler.S_P_BulletsHandler);
        _msgFactories.Add((ushort)MsgId.SPBullets, () => new S_P_Bullets());

        _onRecv.Add((ushort)MsgId.SPRenewScores, UnpackPacket<S_P_RenewScores>);
        _handler.Add((ushort)MsgId.SPRenewScores, PacketHandler.S_P_RenewScoresHandler);
        _msgFactories.Add((ushort)MsgId.SPRenewScores, () => new S_P_RenewScores());

        _onRecv.Add((ushort)MsgId.SPKeepAlive, UnpackPacket<S_P_KeepAlive>);
        _handler.Add((ushort)MsgId.SPKeepAlive, PacketHandler.S_P_KeepAliveHandler);
        _msgFactories.Add((ushort)MsgId.SPKeepAlive, () => new S_P_KeepAlive());

        _onRecv.Add((ushort)MsgId.SPResult, UnpackPacket<S_P_Result>);
        _handler.Add((ushort)MsgId.SPResult, PacketHandler.S_P_ResultHandler);
        _msgFactories.Add((ushort)MsgId.SPResult, () => new S_P_Result());

        _onRecv.Add((ushort)MsgId.SResponseMyRecords, UnpackPacket<S_ResponseMyRecords>);
        _handler.Add((ushort)MsgId.SResponseMyRecords, PacketHandler.S_ResponseMyRecordsHandler);
        _msgFactories.Add((ushort)MsgId.SResponseMyRecords, () => new S_ResponseMyRecords());

        _onRecv.Add((ushort)MsgId.SResponsePublicRecords, UnpackPacket<S_ResponsePublicRecords>);
        _handler.Add((ushort)MsgId.SResponsePublicRecords, PacketHandler.S_ResponsePublicRecordsHandler);
        _msgFactories.Add((ushort)MsgId.SResponsePublicRecords, () => new S_ResponsePublicRecords());

        _onRecv.Add((ushort)MsgId.SGameSceneLoadingProgress, UnpackPacket<S_GameSceneLoadingProgress>);
        _handler.Add((ushort)MsgId.SGameSceneLoadingProgress, PacketHandler.S_GameSceneLoadingProgressHandler);
        _msgFactories.Add((ushort)MsgId.SGameSceneLoadingProgress, () => new S_GameSceneLoadingProgress());

        _onRecv.Add((ushort)MsgId.SMState, UnpackPacket<S_M_State>);
        _handler.Add((ushort)MsgId.SMState, PacketHandler.S_M_StateHandler);
        _msgFactories.Add((ushort)MsgId.SMState, () => new S_M_State());

        _onRecv.Add((ushort)MsgId.SMSetSlotState, UnpackPacket<S_M_SetSlotState>);
        _handler.Add((ushort)MsgId.SMSetSlotState, PacketHandler.S_M_SetSlotStateHandler);
        _msgFactories.Add((ushort)MsgId.SMSetSlotState, () => new S_M_SetSlotState());

        _onRecv.Add((ushort)MsgId.SMResponseHitSlot, UnpackPacket<S_M_ResponseHitSlot>);
        _handler.Add((ushort)MsgId.SMResponseHitSlot, PacketHandler.S_M_ResponseHitSlotHandler);
        _msgFactories.Add((ushort)MsgId.SMResponseHitSlot, () => new S_M_ResponseHitSlot());

        _onRecv.Add((ushort)MsgId.SMRenewScores, UnpackPacket<S_M_RenewScores>);
        _handler.Add((ushort)MsgId.SMRenewScores, PacketHandler.S_M_RenewScoresHandler);
        _msgFactories.Add((ushort)MsgId.SMRenewScores, () => new S_M_RenewScores());

        _onRecv.Add((ushort)MsgId.SMResult, UnpackPacket<S_M_Result>);
        _handler.Add((ushort)MsgId.SMResult, PacketHandler.S_M_ResultHandler);
        _msgFactories.Add((ushort)MsgId.SMResult, () => new S_M_Result());
    }

	public void OnRecvPacket(PacketSession session, ArraySegment<byte> buffer) {
		ushort count = 0;

		ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
		count += 2;
		ushort id = BitConverter.ToUInt16(buffer.Array, buffer.Offset + count);
		count += 2;

		Action<PacketSession, ArraySegment<byte>, ushort> action = null;
		if (_onRecv.TryGetValue(id, out action))
			action?.Invoke(session, buffer, id);
	}

	void UnpackPacket<T>(PacketSession session, ArraySegment<byte> buffer, ushort id) where T : IMessage, new()	{
		T pkt = new T();
		pkt.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);

		// 방법1. 모든 핸들러 함수를 메인스레드에서 실행되도록 유도.
		// 이런 푸쉬짓을 내가 할 리가 없지 ㅋㅋㅋ
		//CustomHandler.Invoke(session, pkt, id);

		// 방법2. 받은 즉시 그 스레드에서 핸들러 함수 실행
		// 이 핸들러에 영향을 받는 모든 object는 이제 멀티스레드를 고려해서 설계되어야함.
		// 메인스레드에서만 정상 동작을 보장하는 메서드는 따로 메인스레드에서 동작하도록
		// 설계 해 주어야함.
		Action<PacketSession, IMessage> act = GetPacketHandler(id);
		act?.Invoke(session, pkt);
	}

	public Action<PacketSession, IMessage> GetPacketHandler(ushort id) {
		Action<PacketSession, IMessage> action = null;
		if (_handler.TryGetValue(id, out action))
			return action;
		return null;
	}

	public bool ByteToIMessage(PacketSession session, byte[] plaintext, ushort msgId) {
		//유효성검사 X. 나중에 유효성 검사 로직 필요.
		if (_msgFactories.TryGetValue(msgId, out Func<IMessage> factory)) {
			IMessage pkt = factory.Invoke();
			pkt.MergeFrom(plaintext);
			Action<PacketSession, IMessage> act = GetPacketHandler(msgId);
			act?.Invoke(session, pkt);
			return true;
		}
		return false;
    }
}
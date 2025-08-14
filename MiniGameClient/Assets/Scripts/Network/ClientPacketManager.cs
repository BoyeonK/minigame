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
	
	public Action<PacketSession, IMessage, ushort> CustomHandler { get; set; }

	//PacketManager 초기화 시, 각 패킷을 다룰 델리게이트들을 초기화해준다.
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

		// 방법1. 모든 핸들러 함수고 메인스레드에서 실행되도록 유도.
		//CustomHandler.Invoke(session, pkt, id);

		// 방법2. 받은 즉시 그 스레드에서 핸들러 함수 실행 (고행)
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
			CustomHandler.Invoke(session, pkt, msgId);
			return true;
		}
		return false;
    }
}
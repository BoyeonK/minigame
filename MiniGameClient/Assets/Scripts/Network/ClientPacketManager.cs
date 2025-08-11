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

	//PacketManager �ʱ�ȭ ��, �� ��Ŷ�� �ٷ� ��������Ʈ���� �ʱ�ȭ���ش�.
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
			action.Invoke(session, buffer, id);
	}

	void UnpackPacket<T>(PacketSession session, ArraySegment<byte> buffer, ushort id) where T : IMessage, new()	{
		T pkt = new T();
		pkt.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);
		//main thread�� �ƴϸ�, unity���� �����ϴ� �޼��带 ������ �� ������ �߻��� �� �ִ�.
		//Unity���� �����ϴ� �޼��带 ����ϴ� �۾��� ���ؼ��� ���� thread���� ������ �� �ֵ��� 
		//���� �����尡 �۾��� �۾� queue�� �о��־�� �Ѵ�.
		CustomHandler.Invoke(session, pkt, id);
	}

	public Action<PacketSession, IMessage> GetPacketHandler(ushort id) {
		Action<PacketSession, IMessage> action = null;
		if (_handler.TryGetValue(id, out action))
			return action;
		return null;
	}

	public bool ByteToIMessage(PacketSession session, byte[] plaintext, ushort msgId) {
		//��ȿ���˻� X. ���߿� ��ȿ�� �˻� ���� �ʿ�.
		if (_msgFactories.TryGetValue(msgId, out Func<IMessage> factory)) {
			IMessage pkt = factory.Invoke();
			pkt.MergeFrom(plaintext);
			CustomHandler.Invoke(session, pkt, msgId);
			return true;
		}
		return false;
    }
}
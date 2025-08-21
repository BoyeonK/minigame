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
	
	//PacketManager �ʱ�ȭ ��, �� ��Ŷ�� �ٷ� ��������Ʈ���� �ʱ�ȭ���ش�.
	//_onRecv�� �����κ��� ���� ���̳ʸ����� ������ ��Ŷ ����� msgId�� ����
	// ���̳ʸ��� ����� protobuf�κ����� ������, protobuf�κ��� �˸°� ĳ�����Ѵ�.
	//_handler�� ĳ���õ� protobuf���뿡 ���� ȣ���� handler�Լ��� �����̴�.
	//_msgFactories�� SEncrypted�� ���� ��ȣȭ�� ���̳ʸ��� �˸��� protobuf�� parse�ϱ� ����
	//factory�Լ��̴�.
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

		// ���1. ��� �ڵ鷯 �Լ��� ���ν����忡�� ����ǵ��� ����.
		// �̷� Ǫ������ ���� �� ���� ���� ������
		//CustomHandler.Invoke(session, pkt, id);

		// ���2. ���� ��� �� �����忡�� �ڵ鷯 �Լ� ����
		// �� �ڵ鷯�� ������ �޴� ��� object�� ���� ��Ƽ�����带 ����ؼ� ����Ǿ����.
		// ���ν����忡���� ���� ������ �����ϴ� �޼���� ���� ���ν����忡�� �����ϵ���
		// ���� �� �־����.
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
		//��ȿ���˻� X. ���߿� ��ȿ�� �˻� ���� �ʿ�.
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
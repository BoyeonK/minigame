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
		
	public Action<PacketSession, IMessage, ushort> CustomHandler { get; set; }

	//PacketManager �ʱ�ȭ ��, �� ��Ŷ�� �ٷ� ��������Ʈ���� �ʱ�ȭ���ش�.
	public void Register() {
		_onRecv.Add((ushort)MsgId.SWelcome, UnpackPacket<S_Welcome>);
		_handler.Add((ushort)MsgId.SWelcome, PacketHandler.S_WelcomeHandler);
	}

	public void OnRecvPacket(PacketSession session, ArraySegment<byte> buffer) {
		ushort count = 0;

		ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
		count += 2;
		ushort id = BitConverter.ToUInt16(buffer.Array, buffer.Offset + count);
		count += 2;

		Action<PacketSession, ArraySegment<byte>, ushort> action = null;
		//��ȣȭ�� ����Ǿ����� �б�ó�� (header�� ���� packetId�� 0�� �ƴѰ��)
		if (id != 0) {
			//��ȣȭ�� ������� �ʾ��� ���, _onRecv���� �ش� packetID�� �ش��ϴ� ��������Ʈ�� ã�� ����.
			if (_onRecv.TryGetValue(id, out action))
				action.Invoke(session, buffer, id);
		} 
		else {
			//TODO : Session�� AES Key�� null�� �ƴ� ����̸鼭, ���ÿ� packetID�� 0�� ���,
			//packetHeader �޺κ��� ���̳ʸ��� ���ؼ� ��ȣȭ�� �����Ͽ� �ùٸ� packet�� �����
			//������ packet�� ������� _onRecv���� ��������Ʈ�� ã�� ����.
        }
	}

	void UnpackPacket<T>(PacketSession session, ArraySegment<byte> buffer, ushort id) where T : IMessage, new()	{
		T pkt = new T();
		pkt.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);
		//TODO :
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
}
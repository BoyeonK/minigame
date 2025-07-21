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

	//PacketManager 초기화 시, 각 패킷을 다룰 델리게이트들을 초기화해준다.
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
		//암호화가 적용되었는지 분기처리 (header에 적힌 packetId가 0이 아닌경우)
		if (id != 0) {
			//암호화가 적용되지 않았을 경우, _onRecv에서 해당 packetID에 해당하는 델리게이트를 찾아 실행.
			if (_onRecv.TryGetValue(id, out action))
				action.Invoke(session, buffer, id);
		} 
		else {
			//TODO : Session의 AES Key가 null이 아닌 경우이면서, 동시에 packetID가 0인 경우,
			//packetHeader 뒷부분의 바이너리에 대해서 복호화를 선행하여 올바른 packet을 만들고
			//복원된 packet을 대상으로 _onRecv에서 델리게이트를 찾아 실행.
        }
	}

	void UnpackPacket<T>(PacketSession session, ArraySegment<byte> buffer, ushort id) where T : IMessage, new()	{
		T pkt = new T();
		pkt.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);
		//TODO :
		//main thread가 아니면, unity에서 제공하는 메서드를 실행할 때 에러가 발생할 수 있다.
		//Unity에서 제공하는 메서드를 사용하는 작업에 대해서는 메인 thread에서 실행할 수 있도록 
		//메인 스레드가 작업할 작업 queue에 밀어주어야 한다.
		CustomHandler.Invoke(session, pkt, id);
	}

	public Action<PacketSession, IMessage> GetPacketHandler(ushort id) {
		Action<PacketSession, IMessage> action = null;
		if (_handler.TryGetValue(id, out action))
			return action;
		return null;
	}
}
using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Threading;
using UnityEngine;

public class ServerSession : PacketSession {
	
	private int _id = 0;
	public int ID {
        get => Volatile.Read(ref _id);
        set => Interlocked.Exchange(ref _id, value);
    }

	private int _matchMakeState = 0;

	public int TrySetMatchMakeState(int gameId)	{
		return Interlocked.CompareExchange(ref _matchMakeState, gameId, 0);
	}

	public void CancelMatchMake(int gameId) {
        C_MatchMakeCancel pkt = new C_MatchMakeCancel();
        pkt.GameId = gameId;
        _matchMakeState = 0;
        Send(pkt);
    }

	public void Send(IMessage packet) {
		string msgName = packet.Descriptor.Name.Replace("_", string.Empty);
		MsgId msgId = (MsgId)Enum.Parse(typeof(MsgId), msgName);

		ushort size = (ushort)packet.CalculateSize();
		byte[] sendBuffer = new byte[size + 4];

		Array.Copy(BitConverter.GetBytes((ushort)(size + 4)), 0, sendBuffer, 0, sizeof(ushort));
		Array.Copy(BitConverter.GetBytes((ushort)msgId), 0, sendBuffer, 2, sizeof(ushort));
		Array.Copy(packet.ToByteArray(), 0, sendBuffer, 4, size);
		Send(new ArraySegment<byte>(sendBuffer));
	}

	public override void OnConnected(EndPoint endPoint)	{
		Debug.Log($"OnConnected : {endPoint}");
		IsConnected = true;
	}

	public override void OnDisconnected(EndPoint endPoint) {
		Debug.Log($"OnDisconnected : {endPoint}");
	}

	public override void OnRecvPacket(ArraySegment<byte> buffer) {
		PacketManager.Instance.OnRecvPacket(this, buffer);
		Debug.Log("Recv!");
	}

	public override void OnSend(int numOfBytes)	{
		//Console.WriteLine($"Transferred bytes: {numOfBytes}");
	}
}
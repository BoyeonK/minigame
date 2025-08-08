using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using UnityEngine;

public class ServerSession : PacketSession {
	
	private int _id = 0;
	public int ID {
		get => _id;
		set { _id = value; }
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

		//CustomHandler.Invoke(Session, IMessage, pktId);
		//인즉, PacketQueue.Instace.Push(pktId, IMessage)가 된다.
		PacketManager.Instance.CustomHandler = (session, imessage, pktId) => {
			PacketQueue.Instance.Push(pktId, imessage);
		};

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
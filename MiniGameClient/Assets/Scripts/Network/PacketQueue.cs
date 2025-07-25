﻿using Google.Protobuf;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//TODO :: IMessage가 아니라 AES방식으로 암호화 된 바이너리를 전송해야 함

public class PacketMessage {
	public ushort Id { get; set; }
	public IMessage Message { get; set; }
}

public class PacketQueue {
	public static PacketQueue Instance { get; } = new PacketQueue();

	Queue<PacketMessage> _packetQueue = new Queue<PacketMessage>();
	object _lock = new object();

	public void Push(ushort id, IMessage packet) {
		lock (_lock) {
			_packetQueue.Enqueue(new PacketMessage() { Id = id, Message = packet });
		}
	}

	public PacketMessage Pop() {
		lock (_lock) {
			if (_packetQueue.Count == 0)
				return null;

			return _packetQueue.Dequeue();
		}
	}

	public List<PacketMessage> PopAll() {
		List<PacketMessage> list = new List<PacketMessage>();

		lock (_lock) {
			while (_packetQueue.Count > 0)
				list.Add(_packetQueue.Dequeue());
		}

		return list;
	}
}
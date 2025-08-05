using System;
using System.Collections.Generic;
using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using UnityEngine;
using Org.BouncyCastle.Crypto;
using Org.BouncyCastle.Crypto.Engines;
using Org.BouncyCastle.Crypto.Modes;
using Org.BouncyCastle.Crypto.Parameters;

class PacketMaker {
    public static C_Encrypted MakeCEncrypted(PacketSession session, IMessage rawPkt, ushort pktId) {
        byte[] aesKey = session.AESKey;
        if (aesKey == null || aesKey.Length == 0) {
            Debug.LogError("dd");
            return new C_Encrypted();
        }
        // iv, ciphertext, tag를 만들어서 protobuf로 직렬화해서

        C_Encrypted pkt = new C_Encrypted();
        return pkt;
    }

    public static C_Welcome MakeCWelcome(byte[] aesKey, string message) {
        return new C_Welcome {
            AesKey = Google.Protobuf.ByteString.CopyFrom(aesKey),
            Message = message
        };
    }

    private static C_Login MakeCLoginInternal(string id, string password) {
        return new C_Login {
            Id = id,
            Password = password,
        };
    }

    public static C_Encrypted MakeCLogin(PacketSession session, string id, string password) {
        C_Login rawPkt = MakeCLoginInternal(id, password);
        C_Encrypted pkt = new C_Encrypted();
        return pkt;
    }
}

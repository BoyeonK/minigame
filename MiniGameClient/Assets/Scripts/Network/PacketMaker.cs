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
using Org.BouncyCastle.Security;

class PacketMaker {
    private static readonly SecureRandom _secureRandom = new SecureRandom();
    private const int GCM_IV_LENGTH_BYTES = 12;
    private const int GCM_TAG_LENGTH_BITS = 128;
    private const int GCM_TAG_LENGTH_BYTES = 16;
    private const float _mEpsilon = 0.001f;

    private static C_Encrypted MakeCEncryptedInternal<T>(PacketSession session, T rawPkt, int pktId) where T : IMessage {
        byte[] aesKey = session.AESKey;
        if (aesKey == null || aesKey.Length == 0) {
            Debug.LogError("Session에 AES Key가 유효하지 않습니다.");
            return new C_Encrypted();
        }

        //Protobuf를 바이너리로
        byte[] plaintext = rawPkt.ToByteArray();

        //초기화 벡터
        byte[] iv = new byte[GCM_IV_LENGTH_BYTES];
        _secureRandom.NextBytes(iv);

        //무결성 검사를 위해 pktId를 추가 인증 데이터로 사용.
        byte[] aad = BitConverter.GetBytes(pktId);

        //BouncyCastle 라이브러리 사용을 위한 객체 초기화
        GcmBlockCipher gcmCipher = new GcmBlockCipher(new AesEngine());
        ICipherParameters parameters = new AeadParameters(new KeyParameter(aesKey), GCM_TAG_LENGTH_BITS, iv, aad);
        gcmCipher.Init(true, parameters);

        //AES방식으로 암호화 수행
        byte[] fullOutput = new byte[gcmCipher.GetOutputSize(plaintext.Length)];
        int len = gcmCipher.ProcessBytes(plaintext, 0, plaintext.Length, fullOutput, 0);
        gcmCipher.DoFinal(fullOutput, len);

        //암호화된 바이너리에서 ciphertext와 tag를 분리
        int tagLen = GCM_TAG_LENGTH_BYTES;
        int cipherLen = fullOutput.Length - tagLen;

        byte[] ciphertext = new byte[cipherLen];
        byte[] tag = new byte[tagLen];

        Array.Copy(fullOutput, 0, ciphertext, 0, cipherLen);
        Array.Copy(fullOutput, cipherLen, tag, 0, tagLen);

        //Protobuf로 직렬화
        C_Encrypted pkt = new C_Encrypted {
            Iv = Google.Protobuf.ByteString.CopyFrom(iv),
            Ciphertext = Google.Protobuf.ByteString.CopyFrom(ciphertext),
            Tag = Google.Protobuf.ByteString.CopyFrom(tag),
            MsgId = pktId,
        };
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
        C_Encrypted pkt = MakeCEncryptedInternal(session, rawPkt, (int)MsgId.CLogin);
        return pkt;
    }

    private static C_CreateAccount MakeCCreateAccountInternal(string id, string password) {
        return new C_CreateAccount {
            Id = id,
            Password = password,
        };
    }

    public static C_Encrypted MakeCCreateAccount(PacketSession session, string id, string password) {
        C_CreateAccount rawPkt = MakeCCreateAccountInternal(id, password);
        C_Encrypted pkt = MakeCEncryptedInternal(session, rawPkt, (int)MsgId.CCreateAccount);
        return pkt;
    }

    private static C_Logout MakeCLogoutInternal(ServerSession session) {
        return new C_Logout {
            Dbid = session.ID
        };
    }

    public static C_Encrypted MakeCLogout(ServerSession session) {
        C_Logout rawPkt = MakeCLogoutInternal(session);
        C_Encrypted pkt = MakeCEncryptedInternal(session, rawPkt, (int)MsgId.CLogout);
        return pkt;
    }

    private static C_MatchmakeRequest MakeCMatchMakeRequestInternal(PacketSession session, int gameId) {
        return new C_MatchmakeRequest { GameId = gameId };
    }

    public static C_Encrypted MakeCMatchMakeRequest(PacketSession session, int gameId) {
        C_MatchmakeRequest rawPkt = MakeCMatchMakeRequestInternal(session, gameId);
        C_Encrypted pkt = MakeCEncryptedInternal(session, rawPkt, (int)MsgId.CMatchmakeRequest);
        return pkt;
    }

    public static C_MatchmakeCancel MakeCMatchMakeCancel(PacketSession session, int gameId) { 
        return new C_MatchmakeCancel { GameId = gameId };
    }

    public static C_MatchmakeKeepAlive MakeCMatchMakeKeepAlive(int gameId, Int64 tick) {
        return new C_MatchmakeKeepAlive { GameId = gameId, SentTimeTick = tick };
    }

    public static C_GameSceneLoadingProgress MakeCGameSceneLoadingProgress(float progressRate) {
        int progressPersent = 0;
        if (Mathf.Abs(0.9f - progressRate) < _mEpsilon) {
            progressPersent = 100;
        }
        else {
            progressPersent = (int)(progressRate * 100);
        }
        Debug.Log($"진행상황 전송. 전송할 패킷의 진행률 : {progressPersent}");
        return new C_GameSceneLoadingProgress { Persentage = progressPersent };
    }
}

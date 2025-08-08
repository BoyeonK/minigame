using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Security.Cryptography;
using UnityEngine;
using Org.BouncyCastle.Crypto;
using Org.BouncyCastle.Crypto.Digests;
using Org.BouncyCastle.Crypto.Encodings;
using Org.BouncyCastle.Crypto.Engines;
using Org.BouncyCastle.Crypto.Modes;
using Org.BouncyCastle.Crypto.Parameters;
using Org.BouncyCastle.Security;

//아래의 Handler함수들은 customHandler를 통해서 main thread에서 '모두' 실행되는 구조이므로
//UnityEngine의 메서드들을 사용하여 작성해도 무방하다.
//다만, 게임이 요구하는 스펙이 커질 경우, 즉 일부 로직을 멀티스레드로 실행할 필요성이 생긴다면
//구조를 조금 바꿀 필요가 있다.
class PacketHandler {
	public static void S_WelcomeHandler(PacketSession session, IMessage packet) {
		S_Welcome sWelcomePacket = packet as S_Welcome;

		if (sWelcomePacket.Gameversion != Managers.GameVersion) {
            Debug.Log($"서버의 게임 버전 : {sWelcomePacket.Gameversion}");
            Debug.Log($"클라이언트의 게임 버전 : {Managers.GameVersion}");
			//현재는 클라이언트를 즉시 종료하지만, 팝업으로 에러메세지를 띄우고
			//이후 종료를 유도하는 쪽이 바람직해 보임.
			Debug.LogError("서버와 클라이언트의 버전이 일치하지 않습니다.");
			Application.Quit();
			return;
		}

		byte[] publicKey = sWelcomePacket.PublicKey.ToByteArray();
		RsaKeyParameters rsaParams = null;
		try	{
			AsymmetricKeyParameter keyParam = PublicKeyFactory.CreateKey(publicKey);
			rsaParams = keyParam as RsaKeyParameters;
			if (rsaParams == null)
				throw new Exception("RSA Key가 아님");
		}
		catch (Exception ex) {
			Debug.LogError($"RSA Key Import 실패: {ex.Message}");
			Application.Quit();
			return;
		}

		byte[] aesKey = new byte[32]; // AES-256
		using (var rng = RandomNumberGenerator.Create())
			rng.GetBytes(aesKey);
		session.AESKey = aesKey;

		byte[] encryptedKey;

		try	{
			var encryptEngine = new OaepEncoding(new RsaEngine(), new Sha256Digest());
			encryptEngine.Init(true, rsaParams);
			encryptedKey = encryptEngine.ProcessBlock(aesKey, 0, aesKey.Length);
		}
		catch (Exception ex) {
			Debug.LogError($"RSA 암호화 실패: {ex.Message}");
			Application.Quit();
			return;
		}

		C_Welcome cWelcomePacket = PacketMaker.MakeCWelcome(encryptedKey, "트랄랄레로트랄랄라");
		Managers.Network.Send(cWelcomePacket);
	}

    public static void S_WelcomeResponseHandler(PacketSession session, IMessage packet) {
		S_WelcomeResponse recvPkt = packet as S_WelcomeResponse;
		string asdf = recvPkt.Message;
		Debug.Log(asdf);
		return;
    }

    public static void S_EncryptedHandler(PacketSession session, IMessage packet) {
		S_Encrypted recvPkt = packet as S_Encrypted;
		byte[] key = session.AESKey; // 32 bytes (AES-256)
		byte[] iv = recvPkt.Iv.ToByteArray();
		byte[] ciphertext = recvPkt.Ciphertext.ToByteArray();
		byte[] tag = recvPkt.Tag.ToByteArray();
		int msgId = recvPkt.MsgId;

		Debug.Log($"{msgId} 복호화 시도");

		byte[] aad = BitConverter.GetBytes(msgId);
		if (!BitConverter.IsLittleEndian) {
			Array.Reverse(aad);
		}

		if (iv.Length != 12 || tag.Length != 16) {
			Debug.LogError($"Invalid IV ({iv.Length}) or Tag ({tag.Length}) length.");
			return;
		}

		byte[] plaintext = new byte[ciphertext.Length];
		try	{
			plaintext = DecryptAesGcmInternal(key, iv, ciphertext, tag, aad);
		}
		catch (Exception ex) {
			Debug.LogError($"복호화 실패: {ex.Message}");
			return;
		}

		Debug.Log($"Message Id = {msgId}");
		if (!(PacketManager.Instance.ByteToIMessage(session, plaintext, (ushort)msgId))) {
			Debug.Log("역직렬화 혹은 Custom Handler에 등록 실패");
        }
	}

	private static byte[] DecryptAesGcmInternal(byte[] key, byte[] iv, byte[] ciphertext, byte[] tag, byte[] aad) {
		var cipher = new GcmBlockCipher(new AesEngine());
		var parameters = new AeadParameters(new KeyParameter(key), 128, iv, aad); // 128 = tag bits
		cipher.Init(false, parameters); // false = decrypt

		Debug.Log("1");

		byte[] encrypted = new byte[ciphertext.Length + tag.Length];
		Buffer.BlockCopy(ciphertext, 0, encrypted, 0, ciphertext.Length);
		Buffer.BlockCopy(tag, 0, encrypted, ciphertext.Length, tag.Length);

		Debug.Log("2");

		byte[] output = new byte[cipher.GetOutputSize(encrypted.Length)];
		int len = cipher.ProcessBytes(encrypted, 0, encrypted.Length, output, 0);
		cipher.DoFinal(output, len);

		return output;
	}

	public static void S_LoginHandler(PacketSession session, IMessage packet) {
		S_Login recvPkt = packet as S_Login;
		switch (recvPkt.ValueCaseCase) {
			case (S_Login.ValueCaseOneofCase.Dbid):
				if (recvPkt.Dbid == 0) {
					Debug.Log("없는 아이디.");
					// 실패
                } else {
					// 정상적인 로그인 성공
					ServerSession ss = session as ServerSession;
					ss.ID = recvPkt.Dbid;
					Debug.Log($"{ss.ID} 내 아이디");
                }
				break;
			case (S_Login.ValueCaseOneofCase.Err):
				string errMsg = recvPkt.Err;
                Debug.Log($"{errMsg}");
				break;
			case (S_Login.ValueCaseOneofCase.None):
				// 값이 할당되지 않았을 때 (ㄹㅇ 버그)
				Debug.Log("매우찐빠");
				break;
		}
	}
}


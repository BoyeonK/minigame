﻿using Google.Protobuf;
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

//아래의 Handler함수들은, 모두 customHandler를 통해서 main thread에서 실행되는 구조이므로
//UnityEngine의 메서드들을 사용하여 작성해도 무방하다.
//다만, 게임이 요구하는 스펙이 커질 경우, 즉 일부 로직을 멀티스레드로 실행할 필요성이 생긴다면
//전체적인 구조를 조금 바꿀 필요가 생긴다.
class PacketHandler {
	public static void S_WelcomeHandler(PacketSession session, IMessage packet) {
		S_Welcome sWelcomePacket = packet as S_Welcome;

		if (sWelcomePacket.Gameversion != Managers.GameVersion)	{
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

		C_Welcome cWelcomePacket = new C_Welcome();
		cWelcomePacket.AesKey = Google.Protobuf.ByteString.CopyFrom(encryptedKey);
		//지금은 고정 문자열을 사용하지만, 나중에는 특정한 생성 로직을 도입할 예정.
		cWelcomePacket.Message = "트랄랄레로트랄랄라";

		Managers.Network.Send(cWelcomePacket);
	}

    public static void S_WelcomeResponseHandler(PacketSession session, IMessage packet) {
		S_WelcomeResponse recvPkt = packet as S_WelcomeResponse;
		string asdf = recvPkt.Message;
		Debug.Log(asdf);
		return;
    }

    public static void S_Encrypted(PacketSession session, IMessage packet) {
		S_Encrypted recvPkt = packet as S_Encrypted;
		byte[] key = session.AESKey; // 32 bytes (AES-256)
		byte[] iv = recvPkt.Iv.ToByteArray();
		byte[] ciphertext = recvPkt.Ciphertext.ToByteArray();
		byte[] tag = recvPkt.Tag.ToByteArray();
        ushort msgId = (ushort)recvPkt.MsgId;

		if (iv.Length != 12 || tag.Length != 16) {
			Debug.LogError($"Invalid IV ({iv.Length}) or Tag ({tag.Length}) length.");
			return;
		}

		byte[] plaintext = new byte[ciphertext.Length];
		try	{
			plaintext = DecryptAesGcm(key, iv, ciphertext, tag);
		}
		catch (Exception ex) {
			Debug.LogError($"복호화 실패: {ex.Message}");
			return;
		}

		Debug.Log($"Message Id = {msgId}");
		if (!(PacketManager.Instance.ByteToIMessage(session, plaintext, msgId))) {
			Debug.Log("역직렬화 혹은 Custom Handler에 등록 실패");
        }
	}

	private static byte[] DecryptAesGcm(byte[] key, byte[] iv, byte[] ciphertext, byte[] tag) {
		var cipher = new GcmBlockCipher(new AesEngine());
		var parameters = new AeadParameters(new KeyParameter(key), 128, iv, null); // 128 = tag bits
		cipher.Init(false, parameters); // false = decrypt

		byte[] encrypted = new byte[ciphertext.Length + tag.Length];
		Buffer.BlockCopy(ciphertext, 0, encrypted, 0, ciphertext.Length);
		Buffer.BlockCopy(tag, 0, encrypted, ciphertext.Length, tag.Length);

		byte[] output = new byte[cipher.GetOutputSize(encrypted.Length)];
		int len = cipher.ProcessBytes(encrypted, 0, encrypted.Length, output, 0);
		cipher.DoFinal(output, len);

		return output;
	}
}


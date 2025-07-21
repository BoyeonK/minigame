using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Security.Cryptography;
using UnityEngine;

//아래의 Handler함수들은, 모두 customHandler를 통해서 main thread에서 실행되는 구조이므로
//UnityEngine의 메서드들을 사용하여 작성해도 무방하다.
//다만, 게임이 요구하는 스펙이 커질 경우, 즉 일부 로직을 멀티스레드로 실행할 필요성이 생긴다면
//전체적인 구조를 조금 바꿀 필요가 생긴다.
class PacketHandler {
	public static void S_WelcomeHandler(PacketSession session, IMessage packet) {
		S_Welcome sWelcomePacket = packet as S_Welcome;

		if (sWelcomePacket.Gameversion != Managers.GameVersion) {
			//현재는 클라이언트를 즉시 종료하지만, 팝업으로 에러메세지를 띄우고
			//이후 종료를 유도하는 쪽이 바람직해 보임.
			Debug.LogError("서버와 클라이언트의 버전이 일치하지 않습니다.");
			Application.Quit();
			return;
        }

		byte[] publicKey = sWelcomePacket.PublicKey.ToByteArray();

		RSA rsa = RSA.Create();
		try {
			rsa.ImportSubjectPublicKeyInfo(publicKey, out _);
		}
		catch (Exception ex) {
			//현재는 클라이언트를 즉시 종료하지만, 팝업으로 에러메세지를 띄우고
			//이후 종료를 유도하는 쪽이 바람직해 보임.
			Debug.LogError($"RSA Key Import 실패: {ex.Message}");
			Application.Quit();
			return;
		}

		byte[] aesKey = new byte[32]; // AES-256
		using (var rng = RandomNumberGenerator.Create())
			rng.GetBytes(aesKey);
		session.AESKey = aesKey;

		byte[] encryptedKey = rsa.Encrypt(aesKey, RSAEncryptionPadding.OaepSHA256);

		C_Welcome cWelcomePacket = new C_Welcome();
		cWelcomePacket.AesKey = Google.Protobuf.ByteString.CopyFrom(encryptedKey);
		//지금은 고정 문자열을 사용하지만, 나중에는 특정한 생성 로직을 도입할 예정.
		cWelcomePacket.Message = "트랄랄레로트랄랄라";

		session.Send(cWelcomePacket.ToByteArray());
	}
}



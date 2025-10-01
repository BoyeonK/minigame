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
using static Define;

//아래의 Handler함수들은 customHandler를 통해서 main thread에서 '모두' 실행되는 구조이므로
//UnityEngine의 메서드들을 사용하여 작성해도 무방하다.
//다만, 게임이 요구하는 스펙이 커질 경우, 즉 일부 로직을 멀티스레드로 실행할 필요성이 생긴다면
//구조를 조금 바꿀 필요가 있다.
class PacketHandler {
	public static void S_WelcomeHandler(PacketSession session, IMessage packet) {
		S_Welcome sWelcomePacket = packet as S_Welcome;

		if (sWelcomePacket.Gameversion != Managers.GameVersion) {
			Managers.ExecuteAtMainThread(() => {
				Debug.Log($"서버의 게임 버전 : {sWelcomePacket.Gameversion}");
				Debug.Log($"클라이언트의 게임 버전 : {Managers.GameVersion}");
				//현재는 클라이언트를 즉시 종료하지만, 팝업으로 에러메세지를 띄우고
				//이후 종료를 유도하는 쪽이 바람직해 보임.
				Debug.LogError("서버와 클라이언트의 버전이 일치하지 않습니다.");
#if UNITY_EDITOR
				UnityEditor.EditorApplication.isPlaying = false;
#endif
				Application.Quit(); 
			});
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
			Managers.ExecuteAtMainThread(() => { 
				Debug.LogError($"RSA Key Import 실패: {ex.Message}");
#if UNITY_EDITOR
				UnityEditor.EditorApplication.isPlaying = false;
#endif
				Application.Quit(); 
			});
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
			Managers.ExecuteAtMainThread(() => { 
				Debug.LogError($"RSA 암호화 실패: {ex.Message}");
#if UNITY_EDITOR
				UnityEditor.EditorApplication.isPlaying = false;
#endif
				Application.Quit(); 
			});
			return;
		}

		C_Welcome cWelcomePacket = PacketMaker.MakeCWelcome(encryptedKey, "직선조명을 없애보았더니 씬의 분위기가 마치 후즈앳더도어처럼 되어버렸다.");
		Managers.Network.Send(cWelcomePacket);
	}

    public static void S_WelcomeResponseHandler(PacketSession session, IMessage packet) {
		S_WelcomeResponse recvPkt = packet as S_WelcomeResponse;
		string recvMessage = recvPkt.Message;
#if UNITY_EDITOR
        Managers.ExecuteAtMainThread(() => {
			Debug.Log(recvMessage);
		});
#endif
		return;
    }

    public static void S_EncryptedHandler(PacketSession session, IMessage packet) {
		S_Encrypted recvPkt = packet as S_Encrypted;
		byte[] key = session.AESKey; // 32 bytes (AES-256)
		byte[] iv = recvPkt.Iv.ToByteArray();
		byte[] ciphertext = recvPkt.Ciphertext.ToByteArray();
		byte[] tag = recvPkt.Tag.ToByteArray();
		int msgId = recvPkt.MsgId;

#if UNITY_EDITOR
		Managers.ExecuteAtMainThread(() => { Debug.Log($"{msgId} 복호화 시도"); });
#endif
		byte[] aad = BitConverter.GetBytes(msgId);
		if (!BitConverter.IsLittleEndian) {
			Array.Reverse(aad);
		}

		if (iv.Length != 12 || tag.Length != 16) {
#if UNITY_EDITOR
			Managers.ExecuteAtMainThread(() => { Debug.LogError($"Invalid IV ({iv.Length}) or Tag ({tag.Length}) length."); });
#endif
			return;
		}

		byte[] plaintext = new byte[ciphertext.Length];
		try	{
			plaintext = DecryptAesGcmInternal(key, iv, ciphertext, tag, aad);
		}
		catch (Exception ex) {
			ex.ToString();
#if UNITY_EDITOR
			Managers.ExecuteAtMainThread(() => { Debug.LogError($"복호화 실패: {ex.Message}"); });
#endif
			return;
		}

		Managers.ExecuteAtMainThread(() => { Debug.Log($"Message Id = {msgId}"); });
		if (!(PacketManager.Instance.ByteToIMessage(session, plaintext, (ushort)msgId))) {
#if UNITY_EDITOR
			Managers.ExecuteAtMainThread(() => { Debug.Log("역직렬화 혹은 Custom Handler에 등록 실패"); });
#endif
        }
	}

	private static byte[] DecryptAesGcmInternal(byte[] key, byte[] iv, byte[] ciphertext, byte[] tag, byte[] aad) {
		var cipher = new GcmBlockCipher(new AesEngine());
		var parameters = new AeadParameters(new KeyParameter(key), 128, iv, aad); // 128 = tag bits
		cipher.Init(false, parameters); // false = decrypt

		byte[] encrypted = new byte[ciphertext.Length + tag.Length];
		Buffer.BlockCopy(ciphertext, 0, encrypted, 0, ciphertext.Length);
		Buffer.BlockCopy(tag, 0, encrypted, ciphertext.Length, tag.Length);

		byte[] output = new byte[cipher.GetOutputSize(encrypted.Length)];
		int len = cipher.ProcessBytes(encrypted, 0, encrypted.Length, output, 0);
		cipher.DoFinal(output, len);

		return output;
	}

	public static void S_LoginHandler(PacketSession session, IMessage packet) {
		S_Login recvPkt = packet as S_Login;
		switch (recvPkt.ValueCaseCase) {
			case (S_Login.ValueCaseOneofCase.Dbid):
				//없는 아이디
				if (recvPkt.Dbid == 0) {
					Managers.ExecuteAtMainThread(() => { 
						Debug.Log("없는 아이디.");
                        Managers.Network.LoginCompleted(2);
                    });
					// 실패
				}
				// 정상적인 로그인 성공
				else {	
					ServerSession ss = session as ServerSession;
					ss.ID = recvPkt.Dbid;
					Managers.ExecuteAtMainThread(() => { 
						Debug.Log($"{ss.ID} 내 아이디");
                        Managers.Network.LoginCompleted(0);
                    });
                }
				break;
			//틀린 비밀번호
			case (S_Login.ValueCaseOneofCase.Err):
				string errMsg = recvPkt.Err;
				Managers.ExecuteAtMainThread(() => { 
					Debug.Log($"{errMsg}");
                    Managers.Network.LoginCompleted(1);
                });
				break;
			// 값이 할당되지 않았을 때 (ㄹㅇ 버그, 서버 문제임)
			case (S_Login.ValueCaseOneofCase.None):
				Managers.ExecuteAtMainThread(() => { Debug.Log("매우찐빠"); });
				break;
		}
	}

	public static void S_CreateAccountHandler(PacketSession session, IMessage packet) {
		S_CreateAccount recvPkt = packet as S_CreateAccount;
		bool isSucceed = recvPkt.Success;
		if (isSucceed) {
			Managers.ExecuteAtMainThread(() => {
				Managers.UI.ShowErrorUIOnlyConfirm("계정 생성에 성공하였습니다.");
			});
		}
		else {
			Managers.ExecuteAtMainThread(() => {
				string err = recvPkt.Err;
                Managers.UI.ShowErrorUIOnlyConfirm(err);
            });
		}
	}

	//TODO : 세션을 초기화하는게 나을 것 같음.
	public static void S_LogoutHandler(PacketSession session, IMessage packet) {
		S_Logout recvPkt = packet as S_Logout;
		bool isSucceed = recvPkt.Success;
		if (isSucceed) {
			Managers.ExecuteAtMainThread(() => {
				Managers.UI.ShowErrorUIOnlyConfirm("성공적으로 로그아웃 되었습니다.");
				Managers.Network.OnLogoutAct?.Invoke();
			});
		}
		else {
			//false를 반환한 경우는, 상당히 거시기한 상황. 게임 종료 유도.
            Managers.ExecuteAtMainThread(() => {
                Managers.UI.ShowErrorUIOnlyConfirm("계정 오류!", () => {
#if UNITY_EDITOR
                    UnityEditor.EditorApplication.isPlaying = false;
#endif
                    Application.Quit();
                });
            });
        }
	}

	public static void S_MatchmakeRequestHandler(PacketSession session, IMessage packet) {
        S_MatchmakeRequest recvPkt = packet as S_MatchmakeRequest;
		if (recvPkt.IsSucceed) {
			Managers.Network.ProcessMatchMake(recvPkt.GameId);
		} else {
            Managers.Network.ProcessMatchMake(recvPkt.GameId, recvPkt.Err);
		}
    }

	public static void S_MatchmakeCancelHandler(PacketSession session, IMessage packet) {
        S_MatchmakeCancel recvPkt = packet as S_MatchmakeCancel;
        if (recvPkt.IsSucceed) {
			Managers.Network.ProcessMatchMakeCancel(recvPkt.GameId);
        }
        else {
            Managers.Network.ProcessMatchMakeCancel(recvPkt.GameId, recvPkt.Err);
        }
    }

	public static void S_MatchmakeKeepAliveHandler(PacketSession session, IMessage packet) { 
		S_MatchmakeKeepAlive recvPkt = packet as S_MatchmakeKeepAlive;
		//내 세션이 찾는 중인 게임이 서버에서 찾았다고 한 게임과 같다면, _state를 변경하고 응답.
		int received = recvPkt.GameId;
		
		//TestCode
		//received = 0;

		if (Managers.Network.ResponseKeepAlive(received)) {
			C_MatchmakeKeepAlive responsePkt = PacketMaker.MakeCMatchMakeKeepAlive(received, recvPkt.SentTimeTick);
			Managers.Network.Send(responsePkt);
		}
	}

	public static void S_ExcludedFromMatchHandler(PacketSession session, IMessage packet) {
		Managers.Network.ResponseExcludedFromMatch();
	}

	public static void S_MatchmakeCompletedHandler(PacketSession session, IMessage packet) { 
		//씬 변경 유도
		S_MatchmakeCompleted recvPkt = packet as S_MatchmakeCompleted;
		Managers.Network.ResponseMatchmakeCompleted(recvPkt.GameId);
	}

	public static void S_GameStartedHandler(PacketSession session, IMessage packet)	{
		S_GameStarted recvPkt = packet as S_GameStarted;
		Managers.Network.ResponseGameStarted(recvPkt.GameId);
	}

	public static void S_TestGameStateHandler(PacketSession session, IMessage packet) {
		Managers.ExecuteAtMainThread(() => { Managers.Network.ProcessTestGameState(packet); });
	}

	public static void S_SpawnGameObjectHandler(PacketSession session, IMessage packet)	{
		S_SpawnGameObject recvPkt = packet as S_SpawnGameObject;
		UnityGameObject serializedObj = recvPkt.Object;
		Managers.ExecuteAtMainThread(() => { Managers.Object.CreateObject(serializedObj); });
	}

	public static void S_EndGameHandler(PacketSession session, IMessage packet) { 
		S_EndGame recvPkt = packet as S_EndGame;
		GameType gameType = IntToGameType(recvPkt.GameId);
		switch (gameType) {
			case (GameType.TestMatch):
				Managers.ExecuteAtMainThread(() => { Managers.Network.ResponseTestGameEnd(); });
				break;
			case (GameType.PingPong):
                Managers.ExecuteAtMainThread(() => { Managers.Network.ResponsePingPongEnd(); });
                break; 
			case (GameType.Danmaku):
                Managers.ExecuteAtMainThread(() => { Managers.Network.ResponseDanmakuEnd(); });
                break;
			default:
				break;
		}
	}
	public static void S_P_StateHandler(PacketSession session, IMessage packet)	{
		Managers.ExecuteAtMainThread(() => { Managers.Network.ProcessPState(packet); });
	}
}


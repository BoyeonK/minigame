using Google.Protobuf;
using Google.Protobuf.Protocol;
using Org.BouncyCastle.Bcpg;
using ServerCore;
using System;
using System.Net;
using System.Threading;
using UnityEngine;
using UnityEngine.InputSystem;
using static Define;

public class NetworkManager {
	ServerSession _session = new ServerSession();
	private int _isTryingConnect = 0;
	//클라이언트 내부에서 사용할 연결 상태 변수.
	//통신에 사용할 연결 상태는 Session에서 따로 관리할 예정. (편의성 측면에서의 반정규화 느낌으루다가)
	//이게 단순 bool값이 아니라 어떤 객체였다면 메모리에 할당한 이후, 그 포인터를 양쪽에서 사용하는 느낌으루다가 만들었을 것.
	private bool _isConnected = false;
	
	//매칭중인 게임 종류
    private GameType _matchGameType = GameType.None;
    private readonly object _matchGameTypeLock = new object();
    //중복 요청을 막기 위한 변수
    private int _isMatchRequesting = 0;
	
    public void Init() { }

	public ServerSession GetSession() {
		return _session;
    }

	public bool IsConnected() {
		return _isConnected;
    }

	public bool IsLogined() {
		if (_session.ID != 0)
			return true;
		return false;
    }

	public void Send(IMessage packet) {
		_session.Send(packet);
	}

	public void TryConnectToServer() {
		//이미 연결되있지도 않으면서, 현재 연결 함수가 작동중이 아닌 경우.
		if (_isConnected == true || Interlocked.CompareExchange(ref _isTryingConnect, 1, 0) != 0) {
			return;
		}

		// DNS (Domain Name System)
		string host = Dns.GetHostName();
		IPHostEntry ipHost = Dns.GetHostEntry(host);

		//이걸 나중에 UI로 받아야겠음.
		IPAddress ipAddr = IPAddress.Parse("192.168.0.7");
		//IPAddress ipAddr = IPAddress.Loopback;

		//IPAddress ipAddr = Array.Find(ipHost.AddressList, a => a.AddressFamily == AddressFamily.InterNetwork);
		//IPAddress ipAddr = ipHost.AddressList[0];
		IPEndPoint endPoint = new IPEndPoint(ipAddr, 7777);
		Debug.Log($"{ipAddr}");

		Connector connector = new Connector();

		connector.Connect(endPoint, () => { return _session; }, 1);
	}

	//이 함수는, 일단 내 설계상 메인스레드에서만 호출할 예정.
	//경쟁상태를 고려하지 않고 만듬.
	//TODO : 이 함수가 서버의 Kill을 통해 실행될 수 있음.
	public void TryDisconnect() {
		if (_isConnected == true) {
			_session.Disconnect();

			//매우 큰 깨달음. 회고할때 반드시 짚고 넘어갈 것
			//
			_session = new ServerSession();
			//

			_isConnected = false;
		}
	}

	public void TryLogin(string id, string password) {
        if (id == "" || password == "") {
            Managers.UI.ShowErrorUIOnlyConfirm("입력값이 잘못되었습니다.", () => { });
            return;
        }

        if (IsConnected() && !(IsLogined())) {
            Debug.Log("로그인 시도");
            C_Encrypted pkt = PacketMaker.MakeCLogin(_session, id, password);
            Send(pkt);
        }
    }

	//지금은 직접 0으로 밀고 결과를 통보하지만,
	//일관성을 위해서 서버 주도적으로 바꿀 필요가 있음.
	public void TryLogout() {
        Debug.Log("로그아웃 시도");
        C_Encrypted pkt = PacketMaker.MakeCLogout(_session);
        Managers.Network.Send(pkt);
        Managers.Network.GetSession().ID = 0;

    }

	public void TryCreateAccount(string id, string pw, string pwc) {
        if (id == "" || pw == "" || pwc == "" || pw != pwc) {
            Managers.UI.ShowErrorUIOnlyConfirm("입력값이 잘못되었습니다.", () => { });
            return;
        }

        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("계정생성 시도");
            C_Encrypted pkt = PacketMaker.MakeCCreateAccount(_session, id, pw);
            Managers.Network.Send(pkt);
        }
    }

	public void TryMatchMake(GameType gameType) {
		lock (_matchGameTypeLock) {
			if (_isMatchRequesting == 1 || _matchGameType != 0) {
				return;
			}
			_isMatchRequesting = 1;
        }
        C_Encrypted pkt = PacketMaker.MakeCMatchMakeRequest(_session, (int)gameType);
        Send(pkt);
    }

	public void ProcessMatchMake(int gameId) {
		lock(_matchGameTypeLock) { 
			_matchGameType = IntToGameType(gameId);
			_isMatchRequesting = 0;
		}
		//TODO : 현재 매칭중이라는 것을 UI로 표시하고, 매칭 취소버튼을 UI로 제공
		Managers.ExecuteAtMainThread(() => {
            Debug.Log($"{gameId}번 게임 매칭 대기열 진입");
            OnMatchmakeRequestSucceedAct.Invoke();
        });
    }

	public void ProcessMatchMake(int gameId, string err) {
		lock (_matchGameTypeLock) {
			_isMatchRequesting = 0;
        }
		Managers.ExecuteAtMainThread(() => { Managers.UI.ShowErrorUIOnlyConfirm(err); });
    }

    public void TryMatchMakeCancel() {
		int gameId = 0;
		lock (_matchGameTypeLock) {
            if (_isMatchRequesting == 1) {
                return;
            }
			if (_matchGameType == GameType.InProcess) {
				Managers.ExecuteAtMainThread(() => {
					Managers.UI.ShowErrorUIOnlyConfirm("잠시 후에 다시 시도해 주세요.");
				});
				return;
			}
			_isMatchRequesting = 1;
            gameId = (int)_matchGameType;
        }
        C_MatchmakeCancel pkt = PacketMaker.MakeCMatchMakeCancel(_session, gameId);
        Send(pkt);
    }

	public void ProcessMatchMakeCancel(int gameId) {
		Debug.Log("받은게 있기는 하다.");
        lock (_matchGameTypeLock) {
            _isMatchRequesting = 0;
            if (_matchGameType != IntToGameType(gameId)) {
				Managers.ExecuteAtMainThread(() => { Debug.Log($"gameId 불일치 ( {_matchGameType} != {IntToGameType(gameId)} )"); });
                return;
            }
                
			_matchGameType = GameType.None;
        }
		Managers.ExecuteAtMainThread(() => {
            Debug.Log($"{gameId}번 게임 매칭 대기열 취소");
            OnMatchmakeCancelSucceedAct.Invoke();
        });  
    }

    public void ProcessMatchMakeCancel(int gameId, string err) {
		lock (_matchGameTypeLock) {
			_isMatchRequesting = 0;
		}
		Managers.ExecuteAtMainThread(() => { Debug.Log($"{gameId}번 게임 매칭 대기열 취소 실패"); });
    }

	public void TrySendLoadingProgressRate(float progressRate) {
		C_GameSceneLoadingProgress pkt = PacketMaker.MakeCGameSceneLoadingProgress(progressRate);
		Send(pkt);
	}

	//KeepAlive handler가 전해준 id가 현재 상태와 일치하는지 확인.
	//일치한 경우, InProcess로 변경하고 true를 리턴
    public bool ResponseKeepAlive(int gameId) {
		lock (_matchGameTypeLock) {
			if ((int)_matchGameType == gameId) {
				_matchGameType = GameType.InProcess;
				Managers.ExecuteAtMainThread(() => { OnResponseKeepAliveAct.Invoke(); }); 
                return true;
			}
		}
		return false;
	}

	public void ResponseMatchmakeCompleted(int gameId) {
		//TODO : TestLoadingScene말고 진짜 LoadingScene쓰기 혹은 gameId마다 다른 GameScene준비하기
		Managers.ExecuteAtMainThread(() => {
			Managers.Scene.LoadSceneWithLoadingScene(IntToGameScene(gameId), Define.Scene.TestLoadingScene);
		});
	}

	public void ResponseGameStarted(int gameId) {
		Managers.ExecuteAtMainThread(() => { OnResponseGameStartedAct.Invoke(); });
	}
	
	//이걸 받은 순간, 이미 서버에서는 대기열 밖으로 밀려난 상황.
	public void ResponseExcludedFromMatch() {
		Managers.ExecuteAtMainThread(() => { OnExcludedFromMatchAct.Invoke(); });
	}

	public void TryRequestGameState(int gameId) {
		C_RequestGameState pkt = PacketMaker.MakeCRequestGameState(gameId);
		Send(pkt);
	}

	public void ProcessTestGameState(IMessage packet) {
        S_TestGameState recvPkt = packet as S_TestGameState;
		foreach(UnityGameObject uObj in recvPkt.Objects) {
			Managers.Object.CreateObject(uObj);
		}
    }

    public void ProcessPState(IMessage packet) {
		BaseScene scene = Managers.Scene.GetCurrentSceneComponent();
		if (scene == null)
			return;

		if (scene is PingPongScene pingPongScene) {
            S_P_State pkt = packet as S_P_State;
            if (pkt == null) return;
            int playerId = pkt.PlayerId;
			pingPongScene.SetId(playerId);
        }
    }

    public void ResponsePRequestPlayerBarPosition(IMessage recvPkt) {
        BaseScene scene = Managers.Scene.GetCurrentSceneComponent();
        if (scene == null)
            return;

        if (scene is PingPongScene pingPongScene) {
			S_P_RequestPlayerBarPosition positionPkt = recvPkt as S_P_RequestPlayerBarPosition;
			if (positionPkt == null) 
				return;
			pingPongScene.RenewPlayerBarPosition(positionPkt);
            Vector3 pos = pingPongScene.GetPlayerBarPosition();
            XYZ serializedPosition = new() {
                X = pos.x,
                Y = pos.y,
                Z = pos.z
            };

			C_P_ResponsePlayerBarPosition pkt = new C_P_ResponsePlayerBarPosition()	{
				Position = serializedPosition
			};

			Send(pkt);
        }
    }

	public void ProcessSPBullet(UnityGameObject serializedBullet, float moveDirX, float moveDirZ, float speed, int lastColider) {
        BaseScene scene = Managers.Scene.GetCurrentSceneComponent();
        if (scene == null)
            return;

        if (scene is PingPongScene pingPongScene) {
            GameObject goBullet = null;
            PingPongBulletController bulletController = null;

            goBullet = Managers.Object.FindByObjectId(serializedBullet.ObjectId);
            if (goBullet == null) {
                goBullet = Managers.Object.CreateObject(serializedBullet);
                if (goBullet == null)  {
                    Debug.LogError($"[ProcessSPBullet] 오브젝트 생성 실패: ObjectId={serializedBullet.ObjectId}");
                    return;
                }
            }

            bulletController = goBullet.GetComponent<PingPongBulletController>();

            if (bulletController == null) {
                Debug.LogError($"[ProcessSPBullet] BulletController를 찾을 수 없습니다! GameObject: {goBullet.name}, ObjectId: {serializedBullet.ObjectId}");
                return;
            }

            Vector3 moveDir = new Vector3(moveDirX, 0, moveDirZ);
			Vector3 position = new Vector3(serializedBullet.Position.X, 0.2f, serializedBullet.Position.Z);
			bulletController.SetPositionVector(position);
            bulletController.SetMoveDir(moveDir);
            bulletController.SetSpeed(speed);
            bulletController.SetLastColider(lastColider);
        }
    }

	public void ResponseSPResult(bool isWinner) {

	}

    public void ProcessDanmakuState(IMessage packet) {

    }

	public void ResponseTestGameEnd() {
        lock (_matchGameTypeLock) {
            _isMatchRequesting = 0;
			_matchGameType = GameType.None;
        }
        OnTestGameEndAct?.Invoke();
	}

    public void ResponsePingPongEnd() {
		OnPingPongEndAct?.Invoke();
    }

    public void ResponseDanmakuEnd() {
		OnDanmakuEndAct?.Invoke();
    }

    public void Update() {

	}

    #region Session의 통신 결과를 Client에게 널리 알릴 델리게이터
    //FM대로하면, private로 선언하고 구독 및 구취하는 함수를 public으로 열어야 함.
    public Action OnConnectedAct;
	public Action OnConnectedFailedAct;
	public Action OnLoginAct;
    public Action OnLogoutAct;
    public Action OnWrongIdAct;
	public Action OnWrongPasswordAct;
	public Action OnMatchmakeRequestSucceedAct;
	public Action OnMatchmakeCancelSucceedAct;
	public Action OnResponseKeepAliveAct;
	public Action OnResponseGameStartedAct;
	public Action OnExcludedFromMatchAct;
	public Action OnProcessRequestGameStateAct;
	public Action OnTestGameEndAct;
    public Action OnPingPongEndAct;
    public Action OnDanmakuEndAct;
    #endregion

    public void ConnectCompleted(bool result) {
		_isConnected = result;
		if (_isTryingConnect == 1)
			_isTryingConnect = 0;
		if (result)	{
			OnConnectedAct.Invoke();
		}
		else {
			OnConnectedFailedAct.Invoke();
		}
	}

	public void LoginCompleted(int result) {
		switch (result) {
			case 0:
				OnLoginAct.Invoke();
				break;
			case 1:
				OnWrongIdAct.Invoke();
				break;
			case 2:
				OnWrongPasswordAct.Invoke();
				break;
		}
	}

	public void MatchmakeCompleted(int gameType) {

	}

    
}

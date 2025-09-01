using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Net;
using System.Threading;
using UnityEngine;
using static Define;

public class NetworkManager {
	ServerSession _session = new ServerSession();
	private int _isTryingConnect = 0;
	//클라이언트 내부에서 사용할 연결 상태 변수.
	//통신에 사용할 연결 상태는 Session에서 따로 관리할 예정. (편의성 측면에서의 반정규화 느낌으루다가)
	//이게 단순 bool값이 아니라 어떤 객체였다면 메모리에 할당한 이후, 그 포인터를 양쪽에서 사용하는 느낌으루다가 만들었을 것.
	private bool _isConnected = false;
	
	//매칭중인 게임 종류
    private int _matchGameType = 0;
	

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

		//IPAddress ipAddr = IPAddress.Parse("192.168.0.8");
		IPAddress ipAddr = IPAddress.Loopback;

		//IPAddress ipAddr = Array.Find(ipHost.AddressList, a => a.AddressFamily == AddressFamily.InterNetwork);
		//IPAddress ipAddr = ipHost.AddressList[0];
		IPEndPoint endPoint = new IPEndPoint(ipAddr, 7777);
		Debug.Log($"{ipAddr}");

		Connector connector = new Connector();

		connector.Connect(endPoint, () => { return _session; }, 1);
	}

	//이 함수는, 일단 내 설계상 메인스레드에서만 호출할 예정.
	//경쟁상태를 고려하지 않고 만듬.
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
		int desired = (int)gameType;
		int matchState = Interlocked.CompareExchange(ref _matchGameType, desired, 0);
        //이전 값이 0이 아니었을 경우 (이미 매칭중이었을 경우) 아무것도 하지 않고 리턴.
        if (matchState != 0) {
			return;
		}

		C_Encrypted pkt = PacketMaker.MakeCMatchMakeRequest(_session, (int)gameType);
		Send(pkt);
    }

    public void CancelMatchMake(int gameId) {
		int matchState = Interlocked.CompareExchange(ref _matchGameType, 0, gameId);
		//이미 다른 스레드에 의해 매칭 종료됨
		if (matchState == gameId) {
			C_MatchmakeCancel pkt = PacketMaker.MakeCMatchMakeCancel(_session, gameId);
			Send(pkt);
		}
    }

    public void Update() {

	}

    #region Packet을 만들어 전송

    #endregion

    #region Session의 통신 결과를 Client에게 널리 알릴 델리게이터
    //FM대로하면, private로 선언하고 구독 및 구취하는 함수를 public으로 열어야 함.
    public Action OnConnectedAct;
	public Action OnConnectedFailedAct;
	public Action OnLoginAct;
    public Action OnLogoutAct;
    public Action OnWrongIdAct;
	public Action OnWrongPasswordAct;

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
	#endregion
}

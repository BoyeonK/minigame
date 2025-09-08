using Google.Protobuf;
using Google.Protobuf.Protocol;
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
	//Ŭ���̾�Ʈ ���ο��� ����� ���� ���� ����.
	//��ſ� ����� ���� ���´� Session���� ���� ������ ����. (���Ǽ� ���鿡���� ������ȭ ��������ٰ�)
	//�̰� �ܼ� bool���� �ƴ϶� � ��ü���ٸ� �޸𸮿� �Ҵ��� ����, �� �����͸� ���ʿ��� ����ϴ� ��������ٰ� ������� ��.
	private bool _isConnected = false;
	
	//��Ī���� ���� ����
    private GameType _matchGameType = GameType.None;
    private readonly object _matchGameTypeLock = new object();
    //�ߺ� ��û�� ���� ���� ����
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

	#region Try�迭 (UI���� ����� ���� ������ ��û�� ����)

	public void TryConnectToServer() {
		//�̹� ����������� �����鼭, ���� ���� �Լ��� �۵����� �ƴ� ���.
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

	//�� �Լ���, �ϴ� �� ����� ���ν����忡���� ȣ���� ����.
	//������¸� ������� �ʰ� ����.
	//TODO : �� �Լ��� ������ Kill�� ���� ����� �� ����.
	public void TryDisconnect() {
		if (_isConnected == true) {
			_session.Disconnect();

			//�ſ� ū ������. ȸ���Ҷ� �ݵ�� ¤�� �Ѿ ��
			//
			_session = new ServerSession();
			//

			_isConnected = false;
		}
	}

	public void TryLogin(string id, string password) {
        if (id == "" || password == "") {
            Managers.UI.ShowErrorUIOnlyConfirm("�Է°��� �߸��Ǿ����ϴ�.", () => { });
            return;
        }

        if (IsConnected() && !(IsLogined())) {
            Debug.Log("�α��� �õ�");
            C_Encrypted pkt = PacketMaker.MakeCLogin(_session, id, password);
            Send(pkt);
        }
    }

	public void TryLogout() {
        Debug.Log("�α׾ƿ� �õ�");
        C_Encrypted pkt = PacketMaker.MakeCLogout(_session);
        Managers.Network.Send(pkt);
        Managers.Network.GetSession().ID = 0;
    }

	public void TryCreateAccount(string id, string pw, string pwc) {
        if (id == "" || pw == "" || pwc == "" || pw != pwc) {
            Managers.UI.ShowErrorUIOnlyConfirm("�Է°��� �߸��Ǿ����ϴ�.", () => { });
            return;
        }

        if (Managers.Network.IsConnected() && !(Managers.Network.IsLogined())) {
            Debug.Log("�������� �õ�");
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
		//TODO : ���� ��Ī���̶�� ���� UI�� ǥ���ϰ�, ��Ī ��ҹ�ư�� UI�� ����
		Managers.ExecuteAtMainThread(() => {
            Debug.Log($"{gameId}�� ���� ��Ī ��⿭ ����");
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
					Managers.UI.ShowErrorUIOnlyConfirm("��� �Ŀ� �ٽ� �õ��� �ּ���.");
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
		Debug.Log("������ �ֱ�� �ϴ�.");
        lock (_matchGameTypeLock) {
            _isMatchRequesting = 0;
            if (_matchGameType != IntToGameType(gameId)) {
				Managers.ExecuteAtMainThread(() => { Debug.Log($"gameId ����ġ ( {_matchGameType} != {IntToGameType(gameId)} )"); });
                return;
            }
                
			_matchGameType = GameType.None;
        }
		Managers.ExecuteAtMainThread(() => {
            Debug.Log($"{gameId}�� ���� ��Ī ��⿭ ���");
            OnMatchmakeCancelSucceedAct.Invoke();
        });  
    }

    public void ProcessMatchMakeCancel(int gameId, string err) {
		lock (_matchGameTypeLock) {
			_isMatchRequesting = 0;
		}
		Managers.ExecuteAtMainThread(() => { Debug.Log($"{gameId}�� ���� ��Ī ��⿭ ��� ����"); });
    }

	//KeepAlive handler�� ������ id�� ���� ���¿� ��ġ�ϴ��� Ȯ��.
	//��ġ�� ���, InProcess�� �����ϰ� true�� ����
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
	
	//�̰� ���� ����, �̹� ���������� ��⿭ ������ �з��� ��Ȳ.
	public void ProcessExcludedFromMatch() {
		Managers.ExecuteAtMainThread(() => { OnExcludedFromMatchAct.Invoke(); });
	}

    public void Update() {

	}

    #endregion

    #region Session�� ��� ����� Client���� �θ� �˸� ����������
    //FM����ϸ�, private�� �����ϰ� ���� �� �����ϴ� �Լ��� public���� ����� ��.
    public Action OnConnectedAct;
	public Action OnConnectedFailedAct;
	public Action OnLoginAct;
    public Action OnLogoutAct;
    public Action OnWrongIdAct;
	public Action OnWrongPasswordAct;
	public Action OnMatchmakeRequestSucceedAct;
	public Action OnMatchmakeCancelSucceedAct;
	public Action OnResponseKeepAliveAct;
	public Action OnExcludedFromMatchAct;

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
	#endregion
}

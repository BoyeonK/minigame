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
	//Ŭ���̾�Ʈ ���ο��� ����� ���� ���� ����.
	//��ſ� ����� ���� ���´� Session���� ���� ������ ����. (���Ǽ� ���鿡���� ������ȭ ��������ٰ�)
	//�̰� �ܼ� bool���� �ƴ϶� � ��ü���ٸ� �޸𸮿� �Ҵ��� ����, �� �����͸� ���ʿ��� ����ϴ� ��������ٰ� ������� ��.
	private bool _isConnected = false;
	
	//��Ī���� ���� ����
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
		int desired = (int)gameType;
		int matchState = Interlocked.CompareExchange(ref _matchGameType, desired, 0);
        //���� ���� 0�� �ƴϾ��� ��� (�̹� ��Ī���̾��� ���) �ƹ��͵� ���� �ʰ� ����.
        if (matchState != 0) {
			return;
		}

		C_Encrypted pkt = PacketMaker.MakeCMatchMakeRequest(_session, (int)gameType);
		Send(pkt);
    }

    public void CancelMatchMake(int gameId) {
		int matchState = Interlocked.CompareExchange(ref _matchGameType, 0, gameId);
		//�̹� �ٸ� �����忡 ���� ��Ī �����
		if (matchState == gameId) {
			C_MatchmakeCancel pkt = PacketMaker.MakeCMatchMakeCancel(_session, gameId);
			Send(pkt);
		}
    }

    public void Update() {

	}

    #region Packet�� ����� ����

    #endregion

    #region Session�� ��� ����� Client���� �θ� �˸� ����������
    //FM����ϸ�, private�� �����ϰ� ���� �� �����ϴ� �Լ��� public���� ����� ��.
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

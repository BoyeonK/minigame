using Google.Protobuf;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;

public class NetworkManager {
	ServerSession _session = new ServerSession();
	private int _isTryingConnect = 0;
	//Ŭ���̾�Ʈ ���ο��� ����� ���� ���� ����.
	//��ſ� ����� ���� ���´� Session���� ���� ������ ����. (���Ǽ� ���鿡���� ������ȭ ��������ٰ�)
	//�̰� �ܼ� bool���� �ƴ϶� � ��ü���ٸ� �޸𸮿� �Ҵ��� ����, �� �����͸� ���ʿ��� ����ϴ� ��������ٰ� ������� ��.
	private bool _isConnected = false;

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

	public void Update() {

	}

	#region Session�� ��� ����� Client���� �θ� �˸� ����������
	//FM����ϸ�, private�� �����ϰ� ���� �� �����ϴ� �Լ��� public���� ����� ��.
	public Action OnConnectedAct;
	public Action OnConnectedFailedAct;
	public Action OnLoginAct;
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

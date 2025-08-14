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
	//클라이언트 내부에서 사용할 연결 상태 변수.
	//통신에 사용할 연결 상태는 Session에서 따로 관리할 예정. (편의성 측면에서의 반정규화 느낌으루다가)
	//이게 단순 bool값이 아니라 어떤 객체였다면 메모리에 할당한 이후, 그 포인터를 양쪽에서 사용하는 느낌으루다가 만들었을 것.
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

	#region Session의 통신 결과를 Client에게 널리 알릴 델리게이터
	//FM대로하면, private로 선언하고 구독 및 구취하는 함수를 public으로 열어야 함.
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

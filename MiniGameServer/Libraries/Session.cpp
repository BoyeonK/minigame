#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

//recvBuffer와 sendBuffer는 이후에 새로운 class로서 만들어 줄 것임.
Session::Session() : _RecvBuffer(BUFFER_SIZE) {
	_socketHandle = SocketUtils::CreateSocket();
}

Session::~Session() {
	SocketUtils::Close(_socketHandle);
}

void Session::Send(shared_ptr<SendBuffer> sendBufferRef) {
	if (isConnected() == false)
		return;
	bool registerSend = false;
	{
		WRITE_RWLOCK;
		_sendBufferRefQueue.push(sendBufferRef);
		//exchange는 인자의 값으로 atomic변수를 바꾸면서
		//원래의 값을 리턴한다.
		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}
	if (registerSend)
		RegisterSend();
}

bool Session::Connect() {
	return RegisterConnect();
}

void Session::Disconnect() {
	//이미 연결이 되어 있지 않은 경우
	if (_connected.exchange(false) == false)
		return;
	RegisterDisconnect();
}

HANDLE Session::GetHandle() {
	return reinterpret_cast<HANDLE>(_socketHandle);
}

void Session::Dispatch(CPTask* pCPTask, int32_t numOfBytes) {
	switch (pCPTask->_TaskType) {
	case (TaskType::Connect):
		ProcessConnect();
		break;
	case (TaskType::Disconnect):
		ProcessDisconnect();
		break;
	case (TaskType::Recv):
		ProcessRecv(numOfBytes);
		break;
	case (TaskType::Send):
		ProcessSend(numOfBytes);
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect() {
	if (isConnected()) 
		return false;
		
	//소유한 service객체가 client용인지 확인

	if (SocketUtils::SetReuseAddress(_socketHandle, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socketHandle, 0) == false)
		return false;

	_CT.Init();
	_CT._OwnerRef = shared_from_this();

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetAddress().GetSockAddr();
	if (false == SocketUtils::ConnectEx(_socketHandle, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_CT)) {
		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
#ifdef _DEBUG
			cout << "ConnectEx함수 실행 에러" << endl;
#endif
			_CT._OwnerRef = nullptr;
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect() {
	_DCT.Init();
	_DCT._OwnerRef = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_socketHandle, &_DCT, TF_REUSE_SOCKET, 0)) {
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			_DCT._OwnerRef = nullptr;
			return false;
		}
	}
	return true;
}

void Session::RegisterRecv() {
	if (isConnected() == false)
		return;

	_RT.Init();
	_RT._OwnerRef = shared_from_this();
	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_RecvBuffer.WritePos());
	wsaBuf.len = _RecvBuffer.FreeSize();
	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socketHandle, &wsaBuf, 1, &numOfBytes, &flags, &_RT, nullptr)) {
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			_RT._OwnerRef = nullptr;
		}
	}
}

void Session::RegisterSend() {
	if (isConnected() == false)
		return;
	_ST.Init();
	_ST._OwnerRef = shared_from_this();
	{
		WRITE_RWLOCK;
		int32_t writeSize = 0;
		while (_sendBufferRefQueue.empty() == false) {
			shared_ptr<SendBuffer> sendBufferRef = _sendBufferRefQueue.front();
			writeSize += sendBufferRef->WriteSize();

			_sendBufferRefQueue.pop();
			_ST._sendBufferRefs.push_back(sendBufferRef);
		}
	}

	vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_ST._sendBufferRefs.size());
	WSABUF wsaBuf;
	for (shared_ptr<SendBuffer>& sendBufferRef : _ST._sendBufferRefs) {
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBufferRef->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBufferRef->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socketHandle, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT &numOfBytes, 0, &_ST, nullptr)) {
		int32_t errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			_ST._OwnerRef = nullptr;
			_ST._sendBufferRefs.clear();
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect() {
	_CT._OwnerRef = nullptr;
	_connected.store(true);
	GetService()->AddSession(GetSessionRef());
	OnConnected();
	RegisterRecv();
}

void Session::ProcessDisconnect() {
	OnDisconnected();
	GetService()->ReleaseSession(GetSessionRef());
	_DCT._OwnerRef = nullptr;
}

void Session::ProcessRecv(int32_t numOfBytes) {
	_RT._OwnerRef = nullptr;
	if (numOfBytes == 0) {
		//연결이 끊겼을 때 0byte Recv가 들어온다.
		cout << "0byte Recv!!" << endl;
		Disconnect();
		return;
	}

	if (_RecvBuffer.OnWrite(numOfBytes) == false) {
		//overflow 발생
		return;
	}

	int32_t dataSize = _RecvBuffer.DataSize();
	int32_t processLen = OnRecv(_RecvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen||_RecvBuffer.OnRead(processLen) == false) {
		//overflow 발생
		return;
	}
	_RecvBuffer.Clean();
	RegisterRecv();
}

void Session::ProcessSend(int32_t numOfBytes) {
	_ST._OwnerRef = nullptr;
	_ST._sendBufferRefs.clear();
	if (numOfBytes == 0) {
		cout << "0bytes sent?" << endl;
	}
	OnSend(numOfBytes);

	bool isEmpty = true;
	{
		WRITE_RWLOCK;
		if (_sendBufferRefQueue.empty())
			_sendRegistered.store(false);
		else
			isEmpty = false;
	}
	if (isEmpty == false)
		RegisterSend();
}

void Session::HandleError(int32_t errorCode) {
}

int32_t PBSession::OnRecv(unsigned char* buffer, int32_t len) {
	//1. 버퍼 1개도 제대로 전송 안 되었을 수 있음.
		//1_1. 헤더조차도 전송될 여지가 없는 바이트
		//1_2. 못해도 헤더는 전송 될 여지가 있다.
	//2. 버퍼 1개는 일단 제대로 전송됨
	//3. 다수의 버퍼가 한번에 전송되었을 가능성 있음 (버퍼 1개처리하고 1번부터 다시시작)
	int32_t processLen = 0;
	while (true) {
		int32_t dataSize = len - processLen;
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		if (dataSize < header._size)
			break;

		OnRecvPacket(&buffer[processLen], header._size);

		processLen += header._size;
	}
	
	return processLen;
}

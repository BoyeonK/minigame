#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

//recvBuffer�� sendBuffer�� ���Ŀ� ���ο� class�μ� ����� �� ����.
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
		//exchange�� ������ ������ atomic������ �ٲٸ鼭
		//������ ���� �����Ѵ�.
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
	//�̹� ������ �Ǿ� ���� ���� ���
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
		
	//������ service��ü�� client������ Ȯ��

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
			cout << "ConnectEx�Լ� ���� ����" << endl;
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
		//������ ������ �� 0byte Recv�� ���´�.
		cout << "0byte Recv!!" << endl;
		Disconnect();
		return;
	}

	if (_RecvBuffer.OnWrite(numOfBytes) == false) {
		//overflow �߻�
		return;
	}

	int32_t dataSize = _RecvBuffer.DataSize();
	int32_t processLen = OnRecv(_RecvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen||_RecvBuffer.OnRead(processLen) == false) {
		//overflow �߻�
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
	//1. ���� 1���� ����� ���� �� �Ǿ��� �� ����.
		//1_1. ��������� ���۵� ������ ���� ����Ʈ
		//1_2. ���ص� ����� ���� �� ������ �ִ�.
	//2. ���� 1���� �ϴ� ����� ���۵�
	//3. �ټ��� ���۰� �ѹ��� ���۵Ǿ��� ���ɼ� ���� (���� 1��ó���ϰ� 1������ �ٽý���)
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

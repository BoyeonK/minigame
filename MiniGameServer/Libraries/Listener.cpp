#include "pch.h"
#include "Listener.h"

Listener::~Listener() {
	SocketUtils::Close(_socketHandle);
	for (auto p : _pAcceptTasks)
		delete p;
}

bool Listener::StartAccept() {
	//������ Service��ü�� ��ȿ���� Ȯ��
	shared_ptr<ServerService> service = _serverServiceWRef.lock();
	if (service == nullptr)
		return false;

	//Listen ���� ����
	_socketHandle = SocketUtils::CreateSocket();
	if (_socketHandle == INVALID_SOCKET) return false;

	//iocp �ڵ��� ��ȿ���� �˻�
	if (service->GetCPCoreRef()->GetHandle() == INVALID_HANDLE_VALUE) return false;

	//iocp�� Listener Socket����
	if (service->GetCPCoreRef()->Register(shared_from_this()) == false)
		return false;

	//������ TIME_WAIT�����϶�, ���� ��Ʈ�� �ٸ� ������ bind��û�� ���
	if (SocketUtils::SetReuseAddress(_socketHandle, true) == false)	return false;

	//TCP ������ ���� ���, ���� �����͸� �����ٰ��ΰ�? (default ������.)
	if (SocketUtils::SetLinger(_socketHandle, 0, 0) == false) return false;

	//Bind�õ�
	if (SocketUtils::Bind(_socketHandle, service->GetAddress()) == false) return false;

	//shared_ptr ����
	service = nullptr;

	//Listen�õ�
	if (SocketUtils::Listen(_socketHandle) == false) 
		return false;

	for (int i = 0; i < _maxSessionCount; i++) {
		AcceptTask* pAcceptTask = new AcceptTask;
		pAcceptTask->_OwnerRef = shared_from_this();
		RegisterAccept(pAcceptTask);
	}

	return true;
}

void Listener::RegisterAccept(AcceptTask* pAcceptTask) {
	shared_ptr<Service> service = _serverServiceWRef.lock();
	shared_ptr<Session> sessionRef = service->CreateSessionRef();
	service = nullptr;

	pAcceptTask->Init();
	pAcceptTask->_sessionRef = sessionRef;
	
	DWORD bytesReceived;
	if (false == SocketUtils::AcceptEx(
		_socketHandle,
		sessionRef->GetSocket(),
		sessionRef->_RecvBuffer.WritePos(),
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&bytesReceived,
		static_cast<LPOVERLAPPED>(pAcceptTask)))
	{
		int32_t errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			RegisterAccept(pAcceptTask);
		}
	}
}

void Listener::CloseSocket() {
	SocketUtils::Close(_socketHandle);
}

HANDLE Listener::GetHandle() {
	return reinterpret_cast<HANDLE>(_socketHandle);
}

void Listener::Dispatch(CPTask* pCpTask, int32_t NumOfBytes) {
	switch (pCpTask->_TaskType) {
	case (TaskType::Accept):
		ProcessAccept(static_cast<AcceptTask*>(pCpTask));
		break;
	default:
		CRASH("Listener�� Accept�̿��� Ÿ���� Dispatch�ൿ");
		break;
	}
}

void Listener::ProcessAccept(AcceptTask* pAcceptTask) {
	shared_ptr<Session> sessionRef = pAcceptTask->_sessionRef;

	//Listener Socket�� ������ �ɼ��� AcceptedSocket�� ���� �õ�
	if (false == SocketUtils::SetUpdateAcceptSocket(sessionRef->GetSocket(), _socketHandle)) {
		sessionRef == nullptr;
		RegisterAccept(pAcceptTask);
		return;
	}
	SOCKADDR_IN sockAddress;
	int32_t sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(sessionRef->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr)) {
		sessionRef == nullptr;
		RegisterAccept(pAcceptTask);
		return;
	}

	sessionRef->SetNetAddress(NetAddress(sockAddress));
	shared_ptr<ServerService> service = _serverServiceWRef.lock();
	if (service == nullptr) {
		sessionRef == nullptr;
		cout << "Invalid server service" << endl;
		return;
	}

	sessionRef->ProcessConnect();
}


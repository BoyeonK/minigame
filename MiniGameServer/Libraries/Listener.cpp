#include "pch.h"
#include "Listener.h"

Listener::~Listener() {
	SocketUtils::Close(_socketHandle);
	for (auto p : _pAcceptTasks)
		delete p;
}

bool Listener::StartAccept() {
	//연동된 Service객체가 유효한지 확인
	shared_ptr<ServerService> service = _serverServiceWRef.lock();
	if (service == nullptr)
		return false;

	//Listen 소켓 생성
	_socketHandle = SocketUtils::CreateSocket();
	if (_socketHandle == INVALID_SOCKET) return false;

	//iocp 핸들이 유효한지 검사
	if (service->GetCPCoreRef()->GetHandle() == INVALID_HANDLE_VALUE) return false;

	//iocp와 Listener Socket연동
	if (service->GetCPCoreRef()->Register(shared_from_this()) == false)
		return false;

	//소켓의 TIME_WAIT상태일때, 같은 포트로 다른 소켓의 bind요청을 허용
	if (SocketUtils::SetReuseAddress(_socketHandle, true) == false)	return false;

	//TCP 연결이 끊긴 경우, 남은 데이터를 보내줄것인가? (default 보낸다.)
	if (SocketUtils::SetLinger(_socketHandle, 0, 0) == false) return false;

	//Bind시도
	if (SocketUtils::Bind(_socketHandle, service->GetAddress()) == false) return false;

	//shared_ptr 해제
	service = nullptr;

	//Listen시도
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
		CRASH("Listener에 Accept이외의 타입의 Dispatch행동");
		break;
	}
}

void Listener::ProcessAccept(AcceptTask* pAcceptTask) {
	shared_ptr<Session> sessionRef = move(pAcceptTask->_sessionRef);

	//Listener Socket과 동일한 옵션을 AcceptedSocket에 전달 시도
	if (false == SocketUtils::SetUpdateAcceptSocket(sessionRef->GetSocket(), _socketHandle)) {
		sessionRef = nullptr;
		RegisterAccept(pAcceptTask);
		return;
	}
	SOCKADDR_IN sockAddress;
	int32_t sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(sessionRef->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr)) {
		sessionRef = nullptr;
		RegisterAccept(pAcceptTask);
		return;
	}

	sessionRef->SetNetAddress(NetAddress(sockAddress));
	shared_ptr<ServerService> service = _serverServiceWRef.lock();
	if (service == nullptr) {
		sessionRef = nullptr;
		cout << "Invalid server service" << endl;
		return;
	}

	sessionRef->ProcessConnect();
}


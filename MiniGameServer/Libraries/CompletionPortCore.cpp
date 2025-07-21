#include "pch.h"
#include "CompletionPortCore.h"

CPTask::CPTask(TaskType tasktype) : _TaskType(tasktype) {
	Init();
}

void CPTask::Init() {
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}


CPCore::CPCore() {
	_handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
#ifdef _DEBUG
	ASSERT_CRASH(_handle != INVALID_HANDLE_VALUE);
#endif
}

CPCore::~CPCore() {
	::CloseHandle(_handle);
}

bool CPCore::Register(shared_ptr<CPObject> CPObjectRef) {
	return ::CreateIoCompletionPort(CPObjectRef->GetHandle(), _handle, 0, 0/*ignored*/);
}

bool CPCore::Dispatch(uint32_t timeoutMs) {
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	CPTask* cpTask = nullptr;

	if (true == ::GetQueuedCompletionStatus(
		_handle,
		OUT & numOfBytes,
		OUT & key,
		OUT reinterpret_cast<LPOVERLAPPED*>(&cpTask),
		timeoutMs)
	) {
		shared_ptr<CPObject> cpObject = cpTask->_OwnerRef;
		cpObject->Dispatch(cpTask, numOfBytes);
	} 
	else {
		int32_t errCode = ::WSAGetLastError();
		switch (errCode) {
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO : ·Î±× Âï±â
			shared_ptr<CPObject> cpObject = cpTask->_OwnerRef;
			cpObject->Dispatch(cpTask, numOfBytes);
			break;
		}
	}

	return true;
}


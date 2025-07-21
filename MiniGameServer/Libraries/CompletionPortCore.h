#pragma once
#include "Session.h"
#include "SendBuffer.h"
#include <vector>

class CPTask;
class Session;

class CPObject : public enable_shared_from_this<CPObject> {
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(CPTask* cptask, int32_t NumOfBytes = 0) abstract;
};

enum class TaskType : uint8_t {
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send,
};

class CPTask : public OVERLAPPED {
public:
	CPTask(TaskType tasktype);
	void Init();

	shared_ptr<CPObject> _OwnerRef;
	TaskType _TaskType;
};

class ConnectTask : public CPTask {
public:
	ConnectTask() : CPTask(TaskType::Connect) { }
};

class DisconnectTask : public CPTask {
public:
	DisconnectTask() : CPTask(TaskType::Disconnect) { }

};

class AcceptTask : public CPTask {
public:
	AcceptTask() : CPTask(TaskType::Accept) { }

	shared_ptr<Session> _sessionRef = nullptr;
};

class RecvTask : public CPTask {
public:
	RecvTask() : CPTask(TaskType::Recv) { }
};

class SendTask : public CPTask {
public:
	SendTask() : CPTask(TaskType::Send) { };

	vector<shared_ptr<SendBuffer>>_sendBufferRefs;
};

class CPCore {
public:
	CPCore();
	~CPCore();

	HANDLE GetHandle() { return _handle; }
	bool Register(shared_ptr<CPObject> CPObjectRef);
	bool Dispatch(uint32_t timeoutMs = INFINITE);

private:
	HANDLE _handle;
};


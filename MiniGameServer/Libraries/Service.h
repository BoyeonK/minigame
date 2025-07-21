#pragma once
#include <functional>
#include <set>
#include "Listener.h"
#include "Session.h"
#include "NetAddress.h"

enum class ServiceType : uint8_t {
	Server,
	Client
};

using SessionFactory = function<shared_ptr<Session>()>;

class Service : public enable_shared_from_this<Service> {
public:
	Service(
		ServiceType type,
		shared_ptr<CPCore>CPCoreRef,
		NetAddress address,
		SessionFactory sessionFactory,
		uint32_t maxSessionCount
	);
	virtual ~Service() { }

	NetAddress GetAddress() { return _address; }
	shared_ptr<CPCore> GetCPCoreRef() { return _CPCoreRef; }
	shared_ptr<Session> CreateSessionRef();
	void AddSession(shared_ptr<Session>sessionRef);
	void ReleaseSession(shared_ptr<Session>sessionRef);
	bool CanStart() { return _sessionFactory != nullptr; }

protected:
	USE_RWLOCK;
	shared_ptr<CPCore> _CPCoreRef;
	NetAddress _address;
	uint32_t _maxSessionCount;
	SessionFactory _sessionFactory;
	set<shared_ptr<Session>> _sessionRefs;
	ServiceType _type;
};

class ClientService : public Service {
public:
	ClientService(
		shared_ptr<CPCore>CPCoreRef,
		NetAddress address,
		SessionFactory sessionFactory,
		uint32_t maxSessionCount = 1
	);
	~ClientService() { }

	bool StartConnect();
};

class ServerService : public Service {
public:
	ServerService(
		shared_ptr<CPCore>CPCoreRef,
		NetAddress address,
		SessionFactory sessionFactory,
		uint32_t maxSessionCount
	);
	~ServerService() { }

	void StartAccept();

private:
	shared_ptr<Listener> _listenerRef;
};


#include "pch.h"
#include "Service.h"
#include "Listener.h"

Service::Service(
	ServiceType type,
	shared_ptr<CPCore>CPCoreRef,
	NetAddress address,
	SessionFactory sessionFactory,
	uint32_t maxSessionCount
) :
	_type(type),
	_CPCoreRef(CPCoreRef),
	_address(address),
	_sessionFactory(sessionFactory),
	_maxSessionCount(maxSessionCount)
{ }

shared_ptr<Session> Service::CreateSessionRef() {
	shared_ptr<Session> sessionRef = _sessionFactory();
	weak_ptr<Service> serviceWRef = shared_from_this();
	sessionRef->SetServiceWRef(serviceWRef);
	if (false == _CPCoreRef->Register(static_pointer_cast<CPObject>(sessionRef))) {
		return nullptr;
	}
	return sessionRef;
}

void Service::AddSession(shared_ptr<Session> sessionRef) {
	WRITE_RWLOCK;
	_sessionRefs.insert(sessionRef);
}

void Service::ReleaseSession(shared_ptr<Session> sessionRef) {
	WRITE_RWLOCK;
	_sessionRefs.erase(sessionRef);
}

ServerService::ServerService(
	shared_ptr<CPCore>CPCoreRef,
	NetAddress address,
	SessionFactory sessionFactory,
	uint32_t maxSessionCount
) :
	Service(ServiceType::Server, CPCoreRef, address, sessionFactory, maxSessionCount)
{
	_listenerRef = make_shared<Listener>(maxSessionCount);
}

void ServerService::StartAccept() {
	if (CanStart() == false)
		return;
	weak_ptr<ServerService> ServerServiceWRef = static_pointer_cast<ServerService>(shared_from_this());
	_listenerRef->SetServerService(ServerServiceWRef);
	_listenerRef->StartAccept();
}

ClientService::ClientService(
	shared_ptr<CPCore> CPCoreRef, 
	NetAddress address, 
	SessionFactory sessionFactory, 
	uint32_t maxSessionCount
) :
	Service(ServiceType::Client, CPCoreRef, address, sessionFactory, maxSessionCount)
{ }

bool ClientService::StartConnect() {
	if (CanStart() == false)
		return false;
	const uint32_t sessionCount = _maxSessionCount;
	for (uint32_t i = 0; i < sessionCount; i++) {
		shared_ptr<Session> sessionRef = CreateSessionRef();
		if (sessionRef->Connect() == false) {
			return false;
		}
	}
	return true;
}


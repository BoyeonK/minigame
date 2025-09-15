#pragma once
#include <mutex>
#include "SendBuffer.h"

extern class ThreadManager* GThreadManager;
extern class SendBufferManager* GSendBufferManager;
extern class GlobalActorQueue* GActorQueue;
extern class ActorEventScheduler* GActorEventScheduler;
extern thread_local uint32_t MyThreadID;
extern thread_local uint64_t LEndTickCount;
extern thread_local shared_ptr<SendBufferChunk> LSendBufferChunkRef;
extern thread_local class Actor* LCurrentActor;

class ThreadManager {
public:
	ThreadManager();
	~ThreadManager();

	static void InitTLS();
	static void DestroyTLS() { };
	static void DoGlobalQueueWork();
	static void DoTimerQueueDistribution();

	void Launch(function<void()> callback);
	void Join();

private:
	mutex	_mutex;
	vector<thread> _threads;
};
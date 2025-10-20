#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "Actor.h"
#include "GlobalActorQueue.h"
#include "ActorEventScheduler.h"

ThreadManager* GThreadManager = nullptr;
SendBufferManager* GSendBufferManager = nullptr;
GlobalActorQueue* GActorQueue = nullptr;
ActorEventScheduler* GActorEventScheduler = nullptr;
thread_local uint32_t MyThreadID = 0;
thread_local uint64_t LEndTickCount = 0;
thread_local shared_ptr<SendBufferChunk> LSendBufferChunkRef = nullptr;
thread_local Actor* LCurrentActor = nullptr;
thread_local mt19937 LRanGen;

class CoreGlobal {
public:
	CoreGlobal() {
		GThreadManager = new ThreadManager();
		GSendBufferManager = new SendBufferManager();
		GActorQueue = new GlobalActorQueue();
		GActorEventScheduler = new ActorEventScheduler();
		SocketUtils::Init();
	}

	~CoreGlobal() {
		delete GThreadManager;
		delete GSendBufferManager;
		delete GActorQueue;
		delete GActorEventScheduler;
		SocketUtils::Clear();
	}
} GCoreGlobal;

ThreadManager::ThreadManager() {
	InitTLS();
}

ThreadManager::~ThreadManager() {
	Join();
}

void ThreadManager::InitTLS() {
	//NxtThreadID는 오로지 InitTLS함수로만 관리되어야함.
	//따라서 static함수 스코프 내에 static변수로 선언
	//이 함수 스코프를 벗어나도 NxtThreadID 변수는 살아있다.
	static atomic<uint32_t> NxtThreadID = 1;
	MyThreadID = NxtThreadID.fetch_add(1);
	random_device rd;
	LRanGen.seed(rd());
}

void ThreadManager::Launch(function<void(void)> callback) {
	lock_guard<mutex> guard(_mutex);
	_threads.push_back(thread([=] {
		InitTLS();
		callback();
		DestroyTLS();
	}));
}

void ThreadManager::Join() {
	for (thread& t : _threads) {
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::DoGlobalQueueWork() {
	while (LCurrentActor == nullptr) {
		/*
		uint64_t now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;
		*/
		shared_ptr<Actor> ActorRef = GActorQueue->Pop();
		if (ActorRef == nullptr)
			break;
		ActorRef->Execute();
	}
}

void ThreadManager::DoTimerQueueDistribution() {
	const uint64_t now = ::GetTickCount64();
	GActorEventScheduler->Distrubute(now);
}

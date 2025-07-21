#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "JobQueue.h"
#include "GlobalQueue.h"
#include "JobTimer.h"

ThreadManager* GThreadManager = nullptr;
SendBufferManager* GSendBufferManager = nullptr;
GlobalQueue* GlobalJobQueue = nullptr;
JobTimer* GJobTimer = nullptr;
thread_local uint32_t MyThreadID = 0;
thread_local uint64_t LEndTickCount = 0;
thread_local shared_ptr<SendBufferChunk> LSendBufferChunkRef = nullptr;
thread_local JobQueue* LCurrentJobQueue = nullptr;

class CoreGlobal {
public:
	CoreGlobal() {
		GThreadManager = new ThreadManager();
		GSendBufferManager = new SendBufferManager();
		GlobalJobQueue = new GlobalQueue();
		GJobTimer = new JobTimer();
		SocketUtils::Init();
	}

	~CoreGlobal() {
		delete GThreadManager;
		delete GSendBufferManager;
		delete GlobalJobQueue;
		delete GJobTimer;
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
	while (LCurrentJobQueue == nullptr) {
		uint64_t now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;
		shared_ptr<JobQueue> jobQueue = GlobalJobQueue->Pop();
		if (jobQueue == nullptr)
			break;
		jobQueue->Execute();
	}
}

void ThreadManager::DoTimerQueueDistribution() {
	const uint64_t now = ::GetTickCount64();
	GJobTimer->Distrubute(now);
}

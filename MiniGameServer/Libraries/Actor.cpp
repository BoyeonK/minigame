#include "pch.h"
#include "GlobalActorQueue.h"

void Actor::Push(shared_ptr<ActorEvent> event, bool isPostOnly) {
	const int32_t prevCount = _eventCount.fetch_add(1);
	_events.Push(event);
	if (prevCount == 0) {
		if (LCurrentActor == nullptr && isPostOnly) {
			Execute();
		}
		else {
			//이 작업을 실행하는 Thread는 이미 어떤 Actor에 대한 처리중이다.
			//혹은 의도적으로 실행하지않고 GlobalQueue에 Push하는것을 목표로 했다.
			GActorQueue->Push(shared_from_this());
		}
	}
}

void Actor::Execute() {
	//이 Actor를 Execute중인 thread는 다른 Actor의 작업을 실행하지 않겠다는 결의?
	bool expected = false;
	if (_isExecuting.compare_exchange_strong(expected, true)) {
		GActorQueue->Push(shared_from_this());
	}

	LCurrentActor = this;
	while (true) {
		vector<shared_ptr<ActorEvent>> events;
		_events.PopAll(events);
		const int32_t jobCount = static_cast<int32_t>(events.size());
		for (int32_t i = 0; i < jobCount; i++) {
			events[i]->Execute();
		}
		if (_eventCount.fetch_sub(jobCount) == jobCount) {
			//해당 Actor의 모든 작업을 끝마침
			LCurrentActor = nullptr;
			_isExecuting = false;
			return;
		}
		const uint64_t now = ::GetTickCount64();
		if (now >= LEndTickCount) {
			LCurrentActor = nullptr;
			GActorQueue->Push(shared_from_this());
			_isExecuting = false;
			break;
		}
	}
}

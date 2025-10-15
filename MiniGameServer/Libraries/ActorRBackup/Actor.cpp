#include "pch.h"
#include "GlobalActorQueue.h"

void Actor::Execute() {
    // 1. 이 액터가 실행 중임을 스레드 로컬 변수에 표시
    if (LCurrentActor != nullptr) {
        Schedule();
        return;
    }

    //다른 Thread에서 Execute 실행 중.
    bool expected = false;
    if (!_isExecuting.compare_exchange_strong(expected, true)) {
        Schedule();
        return;
    }
        
    LCurrentActor = this;

    // 2. 메일박스에 있는 이벤트를 처리
    vector<ActorEvent*> pEvents;
    _mailbox.PopAll(pEvents);
    const int32_t eventCount = static_cast<int32_t>(pEvents.size());
    for (int32_t i = 0; i < eventCount; i++) {
        pEvents[i]->Execute();
        objectPool<ActorEvent>::dealloc(pEvents[i]);
    }

    LCurrentActor = nullptr;
    _isExecuting = false;
}

void Actor::Push(ActorEvent* pEvent) {
    const int32_t prevCount = _eventCount.fetch_add(1);
    _mailbox.Push(pEvent);
    if (prevCount == 0) {
        if (LCurrentActor == nullptr) {
            Execute();
        } 
    }
    else {
        Schedule();
    }
}

void Actor::Schedule() {
    GActorQueue->Push(shared_from_this());
}

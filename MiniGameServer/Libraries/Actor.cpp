#include "pch.h"
#include "GlobalActorQueue.h"

void Actor::Execute() {
    // 1. 이 액터가 실행 중임을 스레드 로컬 변수에 표시
    LCurrentActor = this;

    // 2. 메일박스에 있는 모든 이벤트를 처리
    while (true) {
        ActorEvent* event = nullptr;
        if (_mailbox.TryPop(event) == false) {
            break;
        }

        // 이벤트 실행 및 풀에 반납
        event->Execute();
        objectPool<ActorEvent>::dealloc(event);
    }

    LCurrentActor = nullptr;
    _isScheduled = false;

    //4. 새 이벤트가 들어왔는지 다시 확인
    if (_mailbox._count)
        Schedule();
}

void Actor::Push(ActorEvent* pEvent) {
    _mailbox.Push(pEvent);
    Schedule();
}

void Actor::Schedule() {
    bool expected = false;
    if (_isScheduled.compare_exchange_strong(expected, true)) {
        GActorQueue->Push(shared_from_this());
    }
}

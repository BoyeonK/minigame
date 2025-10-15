#include "pch.h"
#include "GlobalActorQueue.h"

void Actor::Execute() {
    // 1. �� ���Ͱ� ���� ������ ������ ���� ������ ǥ��
    if (LCurrentActor != nullptr) {
        Schedule();
        return;
    }

    //�ٸ� Thread���� Execute ���� ��.
    bool expected = false;
    if (!_isExecuting.compare_exchange_strong(expected, true)) {
        Schedule();
        return;
    }
        
    LCurrentActor = this;

    // 2. ���Ϲڽ��� �ִ� �̺�Ʈ�� ó��
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

#include "pch.h"
#include "GlobalActorQueue.h"

void Actor::Execute() {
    // 1. �� ���Ͱ� ���� ������ ������ ���� ������ ǥ��
    LCurrentActor = this;

    // 2. ���Ϲڽ��� �ִ� ��� �̺�Ʈ�� ó��
    while (true) {
        ActorEvent* event = nullptr;
        if (_mailbox.TryPop(event) == false) {
            break;
        }

        // �̺�Ʈ ���� �� Ǯ�� �ݳ�
        event->Execute();
        objectPool<ActorEvent>::dealloc(event);
    }

    LCurrentActor = nullptr;
    _isScheduled = false;

    //4. �� �̺�Ʈ�� ���Դ��� �ٽ� Ȯ��
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

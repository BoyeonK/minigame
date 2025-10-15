#pragma once

class ActorMailbox {
public:
    ActorMailbox() {
        ActorEvent* dummy = objectPool<ActorEvent>::alloc();
        _head.store(dummy);
        _tail.store(dummy);
    }

    ~ActorMailbox() {
        ActorEvent* out = nullptr;
        while (TryPop(out)) {
            objectPool<ActorEvent>::dealloc(out);
        }
        objectPool<ActorEvent>::dealloc(_head.load());
    }

    void Push(ActorEvent* newEvent) {
        _count.fetch_add(1);
        ActorEvent* prevTail = _tail.exchange(newEvent, memory_order_acq_rel);
        prevTail->next.store(newEvent, memory_order_release);
    }

    bool TryPop(ActorEvent*& outEvent) {
        ActorEvent* head = _head.load(memory_order_relaxed);
        ActorEvent* next = head->next.load(memory_order_acquire);

        if (next == nullptr) {
            return false;
        }

        outEvent = next;
        _head.store(next, memory_order_release);

        _count.fetch_sub(1);
        return true;
    }

    void PopAll(vector<ActorEvent*>& items) {
        ActorEvent* item;
        while (TryPop(item)) {
            items.push_back(item);
        }
    }

    atomic<int> _count = 0;

private:
    atomic<ActorEvent*> _head;
    atomic<ActorEvent*> _tail;
};


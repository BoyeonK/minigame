#pragma once
#include "ActorEvent.h"
#include "ActorEventScheduler.h"
#include "ActorMailbox.h"

class Actor : public enable_shared_from_this<Actor> {
public:
    virtual ~Actor() = default;

    void Post(std::function<void()>&& callback) {
        ActorEvent* pEvent = objectPool<ActorEvent>::alloc(move(callback));
        Push(pEvent);
    }

    void PostAfter(uint64_t tickAfter, function<void()>&& callback) {
        ActorEvent* pEvent = objectPool<ActorEvent>::alloc(move(callback));
        GActorEventScheduler->Reserve(tickAfter, shared_from_this(), pEvent);
    }

    template<typename T, typename Ret, typename... Args>
    void Post(Ret(T::* memFunc)(Args...), Args&&... args) {
        auto myRef = static_pointer_cast<T>(shared_from_this());
        weak_ptr<T> myWRef = myRef;

        ActorEvent* pEvent = objectPool<ActorEvent>::alloc(myWRef, memFunc, forward<Args>(args)...);
        Push(pEvent);
    }

    template<typename T, typename Ret, typename... Args>
    void PostAfter(uint64_t tickAfter, Ret(T::* memFunc)(Args...), Args&&... args) {
        auto myRef = static_pointer_cast<T>(shared_from_this());
        weak_ptr<T> myWRef = myRef;

        ActorEvent* pEvent = objectPool<ActorEvent>::alloc(myWRef, memFunc, forward<Args>(args)...);
        GActorEventScheduler->Reserve(tickAfter, shared_from_this(), pEvent);
    }

    void Execute();
    void Push(ActorEvent* pEvent);

private:
    void Schedule();

protected:
    ActorMailbox _mailbox;
    atomic<int32_t> _eventCount = 0;
    atomic<bool> _isExecuting = false;
};


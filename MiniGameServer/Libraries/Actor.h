#pragma once
#include "ActorEvent.h"
#include "MPSCQueue.h"
#include "ActorEventScheduler.h"

class Actor : public enable_shared_from_this<Actor> {
public:
	virtual ~Actor() = default;

	void DispatchEvent(function<void()>&& callback) {
		Push({ objectPool<ActorEvent>::alloc(std::move(callback)), objectPool<ActorEvent>::dealloc });
	}

	void PostEvent(function<void()>&& callback) {
		Push({ objectPool<ActorEvent>::alloc(std::move(callback)), objectPool<ActorEvent>::dealloc }, true);
	}

	//Ret는 void로 작성해도 될 것 같다.
	template<typename T, typename Ret, typename... Args>
	void DispatchEvent(Ret(T::*memFunc)(Args...), Args&&... args) {
		weak_ptr<T> ownerWRef = static_pointer_cast<T>(shared_from_this());
		Push({ objectPool<ActorEvent>::alloc(ownerWRef, memFunc, forward<Args>(args)...), objectPool<ActorEvent>::dealloc });
	}

	template<typename T, typename Ret, typename... Args>
	void PostEvent(Ret(T::* memFunc)(Args...), Args&&... args) {
		weak_ptr<T> ownerWRef = static_pointer_cast<T>(shared_from_this());
		Push({ objectPool<ActorEvent>::alloc(ownerWRef, memFunc, forward<Args>(args)...), objectPool<ActorEvent>::dealloc }, true);
	}

	//나 이거 왠지 JS에서 써본거같아
	void PostEventAfter(uint64_t tickAfter, function<void()>&& callback) {
		shared_ptr<ActorEvent> eventRef = { objectPool<ActorEvent>::alloc(std::move(callback)), objectPool<ActorEvent>::dealloc };
		GActorEventScheduler->Reserve(tickAfter, shared_from_this(), eventRef);
	}

	template<typename T, typename Ret, typename... Args>
	void PostEventAfter(uint64_t tickAfter, Ret(T::* memFunc)(Args...), Args&&... args) {
		weak_ptr<T> ownerWRef = static_pointer_cast<T>(shared_from_this());
		shared_ptr<ActorEvent> eventRef = { objectPool<ActorEvent>::alloc(ownerWRef, memFunc, forward<Args>(args)...), objectPool<ActorEvent>::dealloc };
		GActorEventScheduler->Reserve(tickAfter, shared_from_this(), eventRef);
	}

	void Execute();
	void Push(shared_ptr<ActorEvent> event, bool isPostOnly = false);

protected:
	MPSCQueue<shared_ptr<ActorEvent>> _events;
	atomic<int32_t> _eventCount = 0;
	atomic<bool> _isExecuting = false;
};


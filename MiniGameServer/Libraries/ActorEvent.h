#pragma once
#include <functional>

class ActorEvent {
public:
	ActorEvent() {}

	explicit ActorEvent(function<void()>&& callback) : _callback(move(callback)) {}

	template<typename T, typename Ret, typename... Args>
	ActorEvent(weak_ptr<T> ownerWRef, Ret(T::* memFunc)(Args...), Args&&... args) {
		auto argsTuple = make_tuple(forward<Args>(args)...);
		_callback = [ownerWRef, memFunc, tup = move(argsTuple)]() mutable {
			shared_ptr<T> owner = ownerWRef.lock();
			if (owner != nullptr)
				apply([&](auto&&... unpacked) {
				(owner.get()->*memFunc)(forward<decltype(unpacked)>(unpacked)...);
					}, move(tup));
			};
	}

	//lambda capture를 통해, shared_ptr을 복사한 경우 문제가 될 수 있다. (되도록 weak_ptr을 쓰려고는 하지만)
	//생명 주기를 보장하기 위해 _callback을 nullptr로 초기화.
	void Execute() {
		if (_callback) {
			_callback();
			_callback = nullptr;
		}
	}

	function<void()> _callback;
	atomic<ActorEvent*> next = nullptr;
};


#pragma once
#include <functional>

class ActorEvent {
public:
	ActorEvent(function<void()>&& callback) : _callback(move(callback)) {}

	//기존 방법은 &&를 사용하긴 한다만, 내부 구현은 완벽 전달을 하지 못함. 람다 캡쳐로 싹 복사 하고 있기 때문.
	/*
	template<typename T, typename Ret, typename... Args>
	Job(weak_ptr<T> ownerWRef, Ret(T::* memFunc)(Args...), Args&&... args)  {
		_callback = [ownerWRef, memFunc, args...]() { 
			shared_ptr<T> owner = ownerWRef.lock();
			if (owner != nullptr)
				(owner.get()->*memFunc)(args...);
		};
	}
	*/

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

private:
	function<void()> _callback;
};


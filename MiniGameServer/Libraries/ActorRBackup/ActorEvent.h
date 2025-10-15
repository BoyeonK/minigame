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

	//lambda capture�� ����, shared_ptr�� ������ ��� ������ �� �� �ִ�. (�ǵ��� weak_ptr�� ������� ������)
	//���� �ֱ⸦ �����ϱ� ���� _callback�� nullptr�� �ʱ�ȭ.
	void Execute() {
		if (_callback) {
			_callback();
			_callback = nullptr;
		}
	}

	function<void()> _callback;
	atomic<ActorEvent*> next = nullptr;
};


#pragma once
#include <functional>

class ActorEvent {
public:
	ActorEvent(function<void()>&& callback) : _callback(move(callback)) {}

	//���� ����� &&�� ����ϱ� �Ѵٸ�, ���� ������ �Ϻ� ������ ���� ����. ���� ĸ�ķ� �� ���� �ϰ� �ֱ� ����.
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

	//lambda capture�� ����, shared_ptr�� ������ ��� ������ �� �� �ִ�. (�ǵ��� weak_ptr�� ������� ������)
	//���� �ֱ⸦ �����ϱ� ���� _callback�� nullptr�� �ʱ�ȭ.
	void Execute() {
		if (_callback) {
			_callback();
			_callback = nullptr;
		}
	}

private:
	function<void()> _callback;
};


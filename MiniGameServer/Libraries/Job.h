#pragma once
#include <functional>

using CallbackType = std::function<void()>;

class Job {
public:
	Job(CallbackType&& callback) : _callback(move(callback)) { }

	template<typename T, typename Ret, typename... Args>
	Job(weak_ptr<T> ownerWRef, Ret(T::* memFunc)(Args...), Args&&... args)  {
		_callback = [ownerWRef, memFunc, args...]() { 
			shared_ptr<T> owner = ownerWRef.lock();
			if (owner != nullptr)
				(owner.get()->*memFunc)(args...);
		};
	}

	//lambda capture�� ����, shared_ptr�� ������ ���
	//���� �ֱ⸦ �����ϱ� ���� _callback�� nullptr�� �ʱ�ȭ.
	void Execute() {
		if (_callback) {
			_callback();
			_callback = nullptr;
		}
	}

private:
	CallbackType _callback;
};


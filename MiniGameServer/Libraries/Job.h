#pragma once
#include <functional>

using CallbackType = std::function<void()>;

class Job {
public:
	Job(CallbackType&& callback) : _callback(move(callback)) { }

	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)  {
		_callback = [owner, memFunc, args...]() { (owner.get()->*memFunc)(args...); };
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


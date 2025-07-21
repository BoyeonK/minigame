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

	//lambda capture를 통해, shared_ptr을 복사한 경우
	//생명 주기를 보장하기 위해 _callback을 nullptr로 초기화.
	void Execute() {
		if (_callback) {
			_callback();
			_callback = nullptr;
		}
	}

private:
	CallbackType _callback;
};


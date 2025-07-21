#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

class JobQueue : public enable_shared_from_this<JobQueue> {
public:
	void DoAsync(CallbackType&& callback) {
		Push({ objectPool<Job>::alloc(std::move(callback)), objectPool<Job>::dealloc });
	}

	//Ret�� void�� �ۼ��ص� �� �� ����.
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args) {
		shared_ptr<T> ownerRef = static_pointer_cast<T>(shared_from_this());
		Push({ objectPool<Job>::alloc(ownerRef, memFunc, std::forward<Args>(args)...), objectPool<Job>::dealloc });
	}

	//�� �̰� ���� JS���� �ẻ�Ű���
	void DoTimerAsync(CallbackType&& callback, uint64_t tickAfter) {
		shared_ptr<Job> job = { objectPool<Job>::alloc(std::move(callback)), objectPool<Job>::dealloc };
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoTimerAsync(Ret(T::* memFunc)(Args...), Args... args, uint64_t tickAfter) {
		shared_ptr<T> ownerRef = static_pointer_cast<T>(shared_from_this());
		shared_ptr<Job>job = { objectPool<Job>::alloc(ownerRef, memFunc, std::forward<Args>(args)...), objectPool<Job>::dealloc };
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	void Execute();
	void Push(shared_ptr<Job> job, bool isPushOnly = false);

protected:
	LockQueue<shared_ptr<Job>> _jobs;
	atomic<int32_t> _jobCount = 0;
};


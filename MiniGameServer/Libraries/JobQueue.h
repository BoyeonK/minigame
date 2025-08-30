#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

class JobQueue : public enable_shared_from_this<JobQueue> {
public:
	void DoAsync(CallbackType&& callback) {
		Push({ objectPool<Job>::alloc(std::move(callback)), objectPool<Job>::dealloc });
	}

	void DoAsyncAfter(CallbackType&& callback) {
		Push({ objectPool<Job>::alloc(std::move(callback)), objectPool<Job>::dealloc }, true);
	}

	//Ret�� void�� �ۼ��ص� �� �� ����.
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args) {
		weak_ptr<T> ownerWRef = static_pointer_cast<T>(shared_from_this());
		Push({ objectPool<Job>::alloc(ownerWRef, memFunc, std::forward<Args>(args)...), objectPool<Job>::dealloc });
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsyncAfter(Ret(T::* memFunc)(Args...), Args... args) {
		weak_ptr<T> ownerWRef = static_pointer_cast<T>(shared_from_this());
		Push({ objectPool<Job>::alloc(ownerWRef, memFunc, std::forward<Args>(args)...), objectPool<Job>::dealloc }, true);
	}

	//�� �̰� ���� JS���� �ẻ�Ű���
	void DoTimerAsync(CallbackType&& callback, uint64_t tickAfter) {
		shared_ptr<Job> job = { objectPool<Job>::alloc(std::move(callback)), objectPool<Job>::dealloc };
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoTimerAsync(uint64_t tickAfter, Ret(T::* memFunc)(Args...), Args... args) {
		weak_ptr<T> ownerWRef = static_pointer_cast<T>(shared_from_this());
		shared_ptr<Job> job = { objectPool<Job>::alloc(ownerWRef, memFunc, std::forward<Args>(args)...), objectPool<Job>::dealloc };
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	void Execute();
	void Push(shared_ptr<Job> job, bool isPushOnly = false);

protected:
	LockQueue<shared_ptr<Job>> _jobs;
	atomic<int32_t> _jobCount = 0;
};


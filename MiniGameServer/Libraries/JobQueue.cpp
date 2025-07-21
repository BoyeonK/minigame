#include "pch.h"
#include "GlobalQueue.h"

void JobQueue::Push(shared_ptr<Job> job, bool isPushOnly) {
	const int32_t prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);
	if (prevCount == 0) {
		if (LCurrentJobQueue == nullptr && isPushOnly) {
			Execute();
		}
		else {
			//이 작업을 실행하는 Thread는 이미 어떤 JobQueue를 처리하고있다.
			//혹은 의도적으로 실행하지않고 GlobalQueue에 Push하는것을 목표로 했다.
			GlobalJobQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::Execute() {
	//이 JobQueue를 실행하는 thread는 다른 JobQueue의 작업을 실행하지 않겠다는 결의?
	LCurrentJobQueue = this;
	while (true) {
		vector<shared_ptr<Job>> jobs;
		_jobs.PopAll(jobs);
		const int32_t jobCount = static_cast<int32_t>(jobs.size());
		for (int32_t i = 0; i < jobCount; i++) {
			jobs[i]->Execute();
		}
		if (_jobCount.fetch_sub(jobCount) == jobCount) {
			//해당 JobQueue의 모든 작업을 끝마침
			LCurrentJobQueue = nullptr;
			return;
		}
		const uint64_t now = ::GetTickCount64();
		if (now >= LEndTickCount) {
			LCurrentJobQueue = nullptr;
			GlobalJobQueue->Push(shared_from_this());
			break;
		}
	}
}

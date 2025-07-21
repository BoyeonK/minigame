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
			//�� �۾��� �����ϴ� Thread�� �̹� � JobQueue�� ó���ϰ��ִ�.
			//Ȥ�� �ǵ������� ���������ʰ� GlobalQueue�� Push�ϴ°��� ��ǥ�� �ߴ�.
			GlobalJobQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::Execute() {
	//�� JobQueue�� �����ϴ� thread�� �ٸ� JobQueue�� �۾��� �������� �ʰڴٴ� ����?
	LCurrentJobQueue = this;
	while (true) {
		vector<shared_ptr<Job>> jobs;
		_jobs.PopAll(jobs);
		const int32_t jobCount = static_cast<int32_t>(jobs.size());
		for (int32_t i = 0; i < jobCount; i++) {
			jobs[i]->Execute();
		}
		if (_jobCount.fetch_sub(jobCount) == jobCount) {
			//�ش� JobQueue�� ��� �۾��� ����ħ
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

#pragma once
#include "JobQueue.h"

class GlobalQueue {
public:
	GlobalQueue() { }
	~GlobalQueue() { }

	void Push(shared_ptr<JobQueue> jobQueue);
	shared_ptr<JobQueue> Pop();

private:
	LockQueue<shared_ptr<JobQueue>> _jobQueues;
};


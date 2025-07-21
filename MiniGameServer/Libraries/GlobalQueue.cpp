#include "pch.h"
#include "GlobalQueue.h"

void GlobalQueue::Push(shared_ptr<JobQueue> jobQueue) {
	_jobQueues.Push(jobQueue);
}

shared_ptr<JobQueue> GlobalQueue::Pop() {
	return _jobQueues.Pop();
}

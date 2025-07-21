#pragma once

struct JobData {
	JobData(weak_ptr<JobQueue> owner, shared_ptr<Job> job) : _owner(owner), _job(job) { }

	weak_ptr<JobQueue> _owner;
	shared_ptr<Job> _job;
};

struct TimerItem {
	bool operator<(const TimerItem& other) const {
		return executeTick > other.executeTick;
	}

	uint64_t executeTick = 0;
	JobData* pJobData = nullptr;
};

class JobTimer {
public:
	void Reserve(uint64_t tickAfter, weak_ptr<JobQueue> owner, shared_ptr<Job> job);
	void Distrubute(uint64_t now);
	void Clear();

private:
	USE_RWLOCK;
	priority_queue<TimerItem> _items;
	atomic<bool> _distributing = false;
};



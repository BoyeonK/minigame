#include "pch.h"
#include "JobTimer.h"

void JobTimer::Reserve(uint64_t tickAfter, weak_ptr<JobQueue> owner, shared_ptr<Job> job) {
	const uint64_t executeTick = ::GetTickCount64() + tickAfter;
	JobData* pjobData = objectPool<JobData>::alloc(owner, job);

	WRITE_RWLOCK;
	_items.push(TimerItem{ executeTick, pjobData });
}

void JobTimer::Distrubute(uint64_t now) {
	//atomic�����̱⶧����, exchange�� true�� ���ϵǸ� ������ �̹� �Լ��� ������ ��Ȳ
	if (_distributing.exchange(true) == true)
		return;

	//Lock�� �Ŵ� �ð��� �ּҷ� �ϱ� ���ؼ�
	vector<TimerItem> onTimeItems;
	{
		WRITE_RWLOCK;
		while (_items.empty() == false) {
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;
			onTimeItems.push_back(timerItem);
			_items.pop();
		}
	}

	for (TimerItem& item : onTimeItems) {
		if (shared_ptr<JobQueue> owner = item.pJobData->_owner.lock())
			owner->Push(item.pJobData->_job);

		objectPool<JobData>::dealloc(item.pJobData);
	}

	_distributing.store(false);
}

void JobTimer::Clear() {
	WRITE_RWLOCK;
	while (_items.empty() == false) {
		const TimerItem& timerItem = _items.top();
		objectPool<JobData>::dealloc(timerItem.pJobData);
		_items.pop();
	}
}

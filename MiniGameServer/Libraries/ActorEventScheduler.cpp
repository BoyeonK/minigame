#include "pch.h"
#include "ActorEventScheduler.h"

void ActorEventScheduler::Reserve(uint64_t tickAfter, weak_ptr<Actor> owner, shared_ptr<ActorEvent> event) {
	const uint64_t executeTick = ::GetTickCount64() + tickAfter;
	SchduledActorEvent* pScheduledEvent = objectPool<SchduledActorEvent>::alloc(owner, event);

	WRITE_RWLOCK;
	_items.push(TimerItem{ executeTick, pScheduledEvent });
}

void ActorEventScheduler::Distrubute(uint64_t now) {
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
		if (shared_ptr<Actor> owner = item.pScheuledActorEvent->_ownerWRef.lock())
			owner->Push(item.pScheuledActorEvent->_actorEvent);

		objectPool<SchduledActorEvent>::dealloc(item.pScheuledActorEvent);
	}

	_distributing.store(false);
}

void ActorEventScheduler::Clear() {
	WRITE_RWLOCK;
	while (_items.empty() == false) {
		const TimerItem& timerItem = _items.top();
		objectPool<SchduledActorEvent>::dealloc(timerItem.pScheuledActorEvent);
		_items.pop();
	}
}

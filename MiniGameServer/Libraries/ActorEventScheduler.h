#pragma once

struct SchduledActorEvent {
	SchduledActorEvent(weak_ptr<Actor> owner, shared_ptr<ActorEvent> event) : _ownerWRef(owner), _actorEvent(event) { }

	weak_ptr<Actor> _ownerWRef;
	shared_ptr<ActorEvent> _actorEvent;
};

struct TimerItem {
	bool operator<(const TimerItem& other) const {
		return executeTick > other.executeTick;
	}

	uint64_t executeTick = 0;
	SchduledActorEvent* pScheuledActorEvent = nullptr;
};

class ActorEventScheduler {
public:
	void Reserve(uint64_t tickAfter, weak_ptr<Actor> owner, shared_ptr<ActorEvent> event);
	void Distrubute(uint64_t now);
	void Clear();

private:
	USE_RWLOCK;
	priority_queue<TimerItem> _items;
	atomic<bool> _distributing = false;
};



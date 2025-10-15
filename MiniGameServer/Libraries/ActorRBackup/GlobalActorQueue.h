#pragma once
#include "Actor.h"
#include "MPSCQueue.h"

class GlobalActorQueue {
public:
	GlobalActorQueue() {}
	~GlobalActorQueue() {}

	void Push(shared_ptr<Actor> actorRef);
	shared_ptr<Actor> Pop();

private:
	LockQueue<shared_ptr<Actor>> _actorRefs;
};


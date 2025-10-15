#pragma once
#include "Actor.h"
#include "MPSCQueue.h"

class GlobalActorQueue {
public:
	GlobalActorQueue() { }
	~GlobalActorQueue() { }

	void Push(shared_ptr<Actor> actor);
	shared_ptr<Actor> Pop();

private:
	MPSCQueue<shared_ptr<Actor>> _actorRefs;
};


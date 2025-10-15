#include "pch.h"
#include "GlobalActorQueue.h"

void GlobalActorQueue::Push(shared_ptr<Actor> jobQueue) {
	_actorRefs.Push(jobQueue);
}

shared_ptr<Actor> GlobalActorQueue::Pop() {
	shared_ptr<Actor> actorRef = nullptr;
	if (_actorRefs.TryPop(actorRef)) {
		return actorRef;
	}
	return nullptr;
}

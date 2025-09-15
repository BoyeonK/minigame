#include "pch.h"
#include "GlobalActorQueue.h"

void GlobalActorQueue::Push(shared_ptr<Actor> jobQueue) {
	_actorRefs.Push(jobQueue);
}

shared_ptr<Actor> GlobalActorQueue::Pop() {
	return _actorRefs.Pop();
}

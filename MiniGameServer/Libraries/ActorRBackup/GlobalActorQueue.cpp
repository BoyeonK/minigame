#include "pch.h"
#include "GlobalActorQueue.h"

void GlobalActorQueue::Push(shared_ptr<Actor> actorRef) {
	_actorRefs.Push(actorRef);
}

shared_ptr<Actor> GlobalActorQueue::Pop() {
	return _actorRefs.Pop();
}

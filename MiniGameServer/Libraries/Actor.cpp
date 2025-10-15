#include "pch.h"
#include "GlobalActorQueue.h"

void Actor::Push(shared_ptr<ActorEvent> event, bool isPostOnly) {
	const int32_t prevCount = _eventCount.fetch_add(1);
	_events.Push(event);
	if (prevCount == 0) {
		if (LCurrentActor == nullptr && isPostOnly) {
			Execute();
		}
		else {
			//�� �۾��� �����ϴ� Thread�� �̹� � Actor�� ���� ó�����̴�.
			//Ȥ�� �ǵ������� ���������ʰ� GlobalQueue�� Push�ϴ°��� ��ǥ�� �ߴ�.
			GActorQueue->Push(shared_from_this());
		}
	}
}

void Actor::Execute() {
	//�� Actor�� Execute���� thread�� �ٸ� Actor�� �۾��� �������� �ʰڴٴ� ����?
	bool expected = false;
	if (_isExecuting.compare_exchange_strong(expected, true)) {
		GActorQueue->Push(shared_from_this());
	}

	LCurrentActor = this;
	while (true) {
		vector<shared_ptr<ActorEvent>> events;
		_events.PopAll(events);
		const int32_t jobCount = static_cast<int32_t>(events.size());
		for (int32_t i = 0; i < jobCount; i++) {
			events[i]->Execute();
		}
		if (_eventCount.fetch_sub(jobCount) == jobCount) {
			//�ش� Actor�� ��� �۾��� ����ħ
			LCurrentActor = nullptr;
			_isExecuting = false;
			return;
		}
		const uint64_t now = ::GetTickCount64();
		if (now >= LEndTickCount) {
			LCurrentActor = nullptr;
			GActorQueue->Push(shared_from_this());
			_isExecuting = false;
			break;
		}
	}
}

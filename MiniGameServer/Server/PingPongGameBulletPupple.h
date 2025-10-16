#pragma once
#include "UnityGameObject.h"

class PingPongGameBulletPupple : public UnityGameObject {
	PingPongGameBulletPupple();
	PingPongGameBulletPupple(float x, float y, float z);
	static shared_ptr<PingPongGameBulletPupple> NewTestGameBullet() {
		return { objectPool<PingPongGameBulletPupple>::alloc(), objectPool<PingPongGameBulletPupple>::dealloc };
	}
	static shared_ptr<PingPongGameBulletPupple> NewTestGameBullet(float x, float y, float z) {
		return { objectPool<PingPongGameBulletPupple>::alloc(x, y, z), objectPool<PingPongGameBulletPupple>::dealloc };
	}

	void Update() override;
};


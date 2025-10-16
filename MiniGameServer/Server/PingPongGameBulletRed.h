#pragma once
#include "UnityGameObject.h"

class PingPongGameBulletRed : public UnityGameObject {
	PingPongGameBulletRed();
	PingPongGameBulletRed(float x, float y, float z);
	static shared_ptr<PingPongGameBulletRed> NewTestGameBullet() {
		return { objectPool<PingPongGameBulletRed>::alloc(), objectPool<PingPongGameBulletRed>::dealloc };
	}
	static shared_ptr<PingPongGameBulletRed> NewTestGameBullet(float x, float y, float z) {
		return { objectPool<PingPongGameBulletRed>::alloc(x, y, z), objectPool<PingPongGameBulletRed>::dealloc };
	}

	void Update() override;
};


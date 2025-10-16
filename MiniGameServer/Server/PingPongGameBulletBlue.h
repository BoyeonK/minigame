#pragma once
#include "UnityGameObject.h"

class PingPongGameBulletBlue : public UnityGameObject {
	PingPongGameBulletBlue();
	PingPongGameBulletBlue(float x, float y, float z);
	static shared_ptr<PingPongGameBulletBlue> NewTestGameBullet() {
		return { objectPool<PingPongGameBulletBlue>::alloc(), objectPool<PingPongGameBulletBlue>::dealloc };
	}
	static shared_ptr<PingPongGameBulletBlue> NewTestGameBullet(float x, float y, float z) {
		return { objectPool<PingPongGameBulletBlue>::alloc(x, y, z), objectPool<PingPongGameBulletBlue>::dealloc };
	}

	void Update() override;
};


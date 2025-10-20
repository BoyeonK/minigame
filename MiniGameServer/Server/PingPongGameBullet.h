#pragma once
#include "UnityGameObject.h"

class PingPongGameBullet : public UnityGameObject {
public:
	void SetVector(float px, float pz, float sx, float sz, float speed);
	void UpdateTick(uint64_t tick);
	void Update() override;

	int32_t _lastColider = -1;
	float _posX = 0;
	float _posZ = 0;
	float _moveDirX = 0;
	float _moveDirZ = 0;
	float _speed = 0;
	uint64_t _updatedTick = 0;
};

class PingPongGameBulletRed : public PingPongGameBullet {
public:
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

class PingPongGameBulletPupple : public PingPongGameBullet {
public:
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

class PingPongGameBulletBlue : public PingPongGameBullet {
public:
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

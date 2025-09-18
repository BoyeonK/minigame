#pragma once
#include "UnityGameObject.h"

class TestGameBullet : public UnityGameObject {
public:
	TestGameBullet();
	TestGameBullet(float x, float y, float z);
	static shared_ptr<TestGameBullet> NewTestGameBullet() {
		return { objectPool<TestGameBullet>::alloc(), objectPool<TestGameBullet>::dealloc };
	}
	static shared_ptr<TestGameBullet> NewTestGameBullet(float x, float y, float z) {
		return { objectPool<TestGameBullet>::alloc(x, y, z), objectPool<TestGameBullet>::dealloc };
	}

	void Update() override;
};
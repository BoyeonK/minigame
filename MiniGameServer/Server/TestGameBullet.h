#pragma once
#include "UnityGameObject.h"

class TestGameBullet : public UnityGameObject {
public:
	TestGameBullet();

	void Update() override;
};

static shared_ptr<TestGameBullet> NewTestGameBullet() {
	return { objectPool<TestGameBullet>::alloc(), objectPool<TestGameBullet>::dealloc };
}
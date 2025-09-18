#include "pch.h"
#include "TestGameBullet.h"

TestGameBullet::TestGameBullet() {
	_objectType = GameObjectType::TestGameBullet;
	_isPool = true;
}

TestGameBullet::TestGameBullet(float x, float y, float z) {
	_objectType = GameObjectType::TestGameBullet;
	_isPool = true;
	_position.SetPosition(x, y, z);
}

void TestGameBullet::Update() {
}

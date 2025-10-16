#include "pch.h"
#include "PingPongGameBulletPupple.h"

PingPongGameBulletPupple::~PingPongGameBulletPupple() {
	_objectType = GameObjectType::PingPongGameBulletPupple;
	_isPool = true;
}

PingPongGameBulletPupple::PingPongGameBulletPupple(float x, float y, float z) {
	_objectType = GameObjectType::PingPongGameBulletPupple;
	_isPool = true;
	_position.SetPosition(x, y, z);
}

void PingPongGameBulletPupple::Update() {
}

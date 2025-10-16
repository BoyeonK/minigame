#include "pch.h"
#include "PingPongGameBulletRed.h"

PingPongGameBulletRed::PingPongGameBulletRed() {
	_objectType = GameObjectType::PingPongGameBulletRed;
	_isPool = true;
}

PingPongGameBulletRed::PingPongGameBulletRed(float x, float y, float z) {
	_objectType = GameObjectType::PingPongGameBulletRed;
	_isPool = true;
	_position.SetPosition(x, y, z);
}

void PingPongGameBulletRed::Update() {

}

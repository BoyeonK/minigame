#include "pch.h"
#include "PingPongGameBulletBlue.h"

PingPongGameBulletBlue::PingPongGameBulletBlue() {
	_objectType = GameObjectType::PingPongGameBulletBlue;
	_isPool = true;
}

PingPongGameBulletBlue::PingPongGameBulletBlue(float x, float y, float z) {
	_objectType = GameObjectType::PingPongGameBulletBlue;
	_isPool = true;
	_position.SetPosition(x, y, z);
}

void PingPongGameBulletBlue::Update() {

}

#include "pch.h"
#include "PingPongGameBullet.h"

void PingPongGameBullet::SetMoveVector(float sx, float sz, float speed) {
	_moveDirX = sx;
	_moveDirZ = sz;
	_speed = speed;
}

void PingPongGameBullet::UpdateTick(uint64_t tick) {
	_updatedTick = tick;
}

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

PingPongGameBulletPupple::PingPongGameBulletPupple() {
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

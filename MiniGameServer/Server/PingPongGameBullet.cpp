#include "pch.h"
#include "PingPongGameBullet.h"

void PingPongGameBullet::SetVector(float px, float pz, float sx, float sz, float speed) {
	_posX = px;
	_posZ = pz;
	_moveDirX = sx;
	_moveDirZ = sz;
	_speed = speed;
}

void PingPongGameBullet::UpdateTick(uint64_t tick) {
	_updatedTick = tick;
}

S2C_Protocol::S_P_Bullet PingPongGameBullet::SerializeBullet() const {
	S2C_Protocol::S_P_Bullet outPkt;
	S2C_Protocol::UnityGameObject* bullet_ptr = outPkt.mutable_bullet();
	bullet_ptr->set_objectid(GetObjectId());
	bullet_ptr->set_objecttype(GetObjectTypeInteger());
	S2C_Protocol::XYZ* pos_ptr = bullet_ptr->mutable_position();
	pos_ptr->set_x(_posX);
	pos_ptr->set_y(0.2f);
	pos_ptr->set_z(_posZ);

	S2C_Protocol::XYZ* moveDir_ptr = outPkt.mutable_movedir();
	moveDir_ptr->set_x(_moveDirX);
	moveDir_ptr->set_z(_moveDirZ);

	outPkt.set_speed(_speed);
	outPkt.set_lastcollider(-1);
	return outPkt;
}

void PingPongGameBullet::Update() {
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

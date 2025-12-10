#include "pch.h"
#include "RacePlayer.h"

RacePlayer::RacePlayer() {
	_objectType = GameObjectType::RacePlayer;
	_isPool = true;
}

RacePlayer::RacePlayer(float x, float y, float z) {
	_objectType = GameObjectType::RacePlayer;
	_isPool = true;
	_position.SetPosition(x, y, z);
}

void RacePlayer::Update() {
}

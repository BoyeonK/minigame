#include "pch.h"
#include "GameType.h"

GameType IntToGameType(int32_t num) {
	switch (num) {
	case(0):
		return GameType::None;
	case(1):
		return GameType::TestGame;
	case(2):
		return GameType::PingPong;
	case(3):
		return GameType::Danmaku;
	default:
		return GameType::Undefined;
	}
}

GameObjectType IntToGameObjectType(int32_t num) {
	switch (num) {
	case(1):
		return GameObjectType::TestGameBullet;
	case(2):
		return GameObjectType::PingPongGameBulletRed;
	case(3):
		return GameObjectType::PingPongGameBulletBlue;
	case(4):
		return GameObjectType::PingPongGameBulletPupple;
	default:
		return GameObjectType::undefined;
	}
}

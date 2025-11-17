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
		return GameType::Mole;
	default:
		return GameType::Undefined;
	}
}

GameObjectType IntToGameObjectType(int32_t num) {
	switch (num) {
	case(1):
		return GameObjectType::TestGameBullet;
	case(2):
		return GameObjectType::MyPlayerBar;
	case(3):
		return GameObjectType::EnemyPlayerBar;
	case(4):
		return GameObjectType::PingPongGameBulletRed;
	case(5):
		return GameObjectType::PingPongGameBulletBlue;
	case(6):
		return GameObjectType::PingPongGameBulletPupple;
	default:
		return GameObjectType::undefined;
	}
}

#pragma once

enum class GameType {
	None,
	TestGame,
	PingPong,
	Mole,
	Undefined,
};

GameType IntToGameType(int32_t num); 

enum class GameObjectType {
	undefined,
	TestGameBullet,
	MyPlayerBar,
	EnemyPlayerBar,
	PingPongGameBulletRed,
	PingPongGameBulletBlue,
	PingPongGameBulletPupple,
};

GameObjectType IntToGameObjectType(int32_t num);
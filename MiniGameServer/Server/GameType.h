#pragma once

enum class GameType {
	None,
	TestGame,
	PingPong,
	Danmaku,
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
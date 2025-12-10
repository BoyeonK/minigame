#pragma once

enum class GameType {
	None,
	Race,
	PingPong,
	Mole,
	Undefined,
};

GameType IntToGameType(int32_t num); 

enum class GameObjectType {
	undefined,
	RacePlayer,
	RaceOpponent,
	TestGameBullet,
	MyPlayerBar,
	EnemyPlayerBar,
	PingPongGameBulletRed,
	PingPongGameBulletBlue,
	PingPongGameBulletPupple,
};

GameObjectType IntToGameObjectType(int32_t num);
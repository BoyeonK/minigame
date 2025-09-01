#pragma once

enum class GameType {
	None,
	TestMatch,
	PingPong,
	Danmaku,
	Undefined,
};

GameType IntToGameType(int32_t num); 
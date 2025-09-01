#include "pch.h"
#include "GameType.h"

GameType IntToGameType(int32_t num) {
	switch (num) {
	case(0):
		return GameType::None;
	case(1):
		return GameType::TestMatch;
	case(2):
		return GameType::PingPong;
	case(3):
		return GameType::Danmaku;
	default:
		return GameType::Undefined;
	}
}

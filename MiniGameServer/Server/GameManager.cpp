#include "pch.h"
#include "GameManager.h"

void PingPongManager::Push(WatingPlayerData&& pd) {
	_matchQueue.Push(move(pd));
}
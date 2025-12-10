#pragma once
#include "UnityGameObject.h"

class RacePlayer : public UnityGameObject {
public:
	RacePlayer();
	RacePlayer(float x, float y, float z);
	static shared_ptr<RacePlayer> NewRacePlayer() {
		return { objectPool<RacePlayer>::alloc(), objectPool<RacePlayer>::dealloc };
	}
	static shared_ptr<RacePlayer> NewRacePlayer(float x, float y, float z) {
		return { objectPool<RacePlayer>::alloc(x, y, z), objectPool<RacePlayer>::dealloc };
	}

	void Update() override;
};


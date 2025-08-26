#pragma once
class GameRoom {
	virtual void Update() = 0;
};

class PingPongGameRoom : public GameRoom {
	void Update() override {

	}
};
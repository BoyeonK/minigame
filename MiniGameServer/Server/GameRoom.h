#pragma once
class GameRoom : public JobQueue {
	virtual void Update() = 0;
};

class PingPongGameRoom : public GameRoom {
	void Update() override {

	}
};
#pragma once

class PlayerSession : public PBSession {
public:
	//TODO:
	void OnConnected();
	void OnDisconnected();
	void OnRecvPacket(unsigned char* buffer, int32_t len) override;

private:
	EVP_PKEY* _RSAKey;
	int32_t _gameVersion = 0;
};


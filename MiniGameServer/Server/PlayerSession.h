#pragma once

class PlayerSession : public PBSession {
public:
	//TODO:
	void OnConnected();
	void OnDisconnected();
	void OnRecvPacket(unsigned char* buffer, int32_t len) override;

	EVP_PKEY* GetRSAKey();
	void SetAESKey(vector<unsigned char>& AESKey);

private:
	EVP_PKEY* _RSAKey;
	vector<unsigned char> _AESKey;
	int32_t _gameVersion = 0;
};


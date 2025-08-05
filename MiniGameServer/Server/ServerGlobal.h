#pragma once
#include <mutex>
#include <unordered_map>
#include <cstring>
#include "DBClientImpl.h"
#include "ServerServiceImpl.h"

//extern class ObjectManager* GObjectManager;
extern class CryptoManager* GCryptoManager;
extern class DBClientImpl* DBManager;
extern class shared_ptr<ServerServiceImpl> GServerService;

//높은 확률로 S2C 컨텐츠로 옮겨질 예정.
/*
class ObjectManager {
public:
	
private:
	USE_RWLOCK;
	static atomic<uint32_t> _objectId;
};
*/

class CryptoManager {
public:
	CryptoManager();
	~CryptoManager();

	EVP_PKEY* PopKey();
	bool ReturnKey(EVP_PKEY*& key);

	static vector<unsigned char> ExtractPublicKey(EVP_PKEY* key);
	static vector<unsigned char> Decrypt(EVP_PKEY* privateKey, const vector<unsigned char>& encrypted);
	static bool Encrypt(
		const vector<unsigned char>& key, 
		const vector<unsigned char>& plaintext, 
		const int32_t pktId, 
		vector<unsigned char>& iv, 
		vector<unsigned char>& ciphertext, 
		vector<unsigned char>& tag);


private:
	USE_RWLOCK;
	queue<EVP_PKEY*> _keyQueue;

	//숫자를 세는 것에 더해서 unordered set을 사용하여
	//사용, 비사용된 key를 정확히 추적할 수도 있음
	//나중에 수요가 생기면 추가하는 걸로 하고 넘어간다.
	atomic<uint32_t> _inPool;
	atomic<uint32_t> _outPool;
};
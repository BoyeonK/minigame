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

//���� Ȯ���� S2C �������� �Ű��� ����.
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

	//���ڸ� ���� �Ϳ� ���ؼ� unordered set�� ����Ͽ�
	//���, ����� key�� ��Ȯ�� ������ ���� ����
	//���߿� ���䰡 ����� �߰��ϴ� �ɷ� �ϰ� �Ѿ��.
	atomic<uint32_t> _inPool;
	atomic<uint32_t> _outPool;
};
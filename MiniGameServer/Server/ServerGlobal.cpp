#include "pch.h"
#include "ServerGlobal.h"

ObjectManager* GObjectManager = nullptr;
RSAKeyManager* RSAManager = nullptr;

class ServerGlobal {
public:
	ServerGlobal() {
		GObjectManager = new ObjectManager();
		RSAManager = new RSAKeyManager();
	}
	~ServerGlobal() {
		delete GObjectManager;
		delete RSAManager;
	}
} GServerGlobal;

RSAKeyManager::RSAKeyManager() {
	//최초, 메인스레드에서 1번 실행 될 것이기 때문에
	//생성자 안에서만큼은 multi thread환경을 고려하지 않고 작성됨.
	for (int i = 0; i < 100; i++) {
		EVP_PKEY* pkey = nullptr;
		EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
		if (!ctx) {
			cout << i << "번째 ctx 생성 오류" << endl;
			continue;
		}

		if (EVP_PKEY_keygen_init(ctx) <= 0) {
			cout << i << "번째 keygen init 오류" << endl;
			EVP_PKEY_CTX_free(ctx);
			continue;
		}

		if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
			cout << i << "번째 key size set 오류" << endl;
			EVP_PKEY_CTX_free(ctx);
			continue;
		}

		if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
			cout << i << "번째 pkey 생성 오류" << endl;
			EVP_PKEY_CTX_free(ctx);
			continue;
		}

		EVP_PKEY_CTX_free(ctx);
		_keyQueue.push(pkey);
	}
	_inPool = 100;
	_outPool = 0;
	cout << "RSAKeyManager Initiated" << endl;
}

RSAKeyManager::~RSAKeyManager() {
	//TODO : 사용되고 있는 모든 RSA Key를 회수하고 나서 실행되어야 함.
	//따라서 _outPool이 0인지 확인하는 로직을 추가할 예정.

	WRITE_RWLOCK;
	
	while (!_keyQueue.empty()) {
		EVP_PKEY* key = _keyQueue.front();
		_keyQueue.pop();
		EVP_PKEY_free(key);
		_inPool -= 1;
	}
}

EVP_PKEY* RSAKeyManager::PopKey() {
	
	EVP_PKEY* key = nullptr;
	{
		WRITE_RWLOCK;
		if (!_keyQueue.empty()) {
			key = _keyQueue.front();
			_keyQueue.pop();
			_inPool -= 1;
			_outPool += 1;
			return key;
		}
	}
	
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
	if (!ctx) {
		cout << "ctx 생성 오류" << endl;
		return nullptr;
	}

	if (EVP_PKEY_keygen_init(ctx) <= 0) {
		cout << "Keygen init 오류" << endl;
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}

	if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
		cout << "Key size set 오류" << endl;
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}

	if (EVP_PKEY_keygen(ctx, &key) <= 0) {
		cout << "Key 생성 오류" << endl;
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}

	EVP_PKEY_CTX_free(ctx);
	_outPool += 1;
	return key;
}

//이럴 줄 알았으면 처음부터 _keyQueue를 unique_ptr을 다루는 큐로서 만들 걸 그랬다.
bool RSAKeyManager::ReturnKey(EVP_PKEY*& key) {
	if (!key) {
		cout << "??도대체 뭘 리턴한거지??" << endl;
		return false;
	}
	{
		WRITE_RWLOCK;
		_keyQueue.push(key);
	}
	key = nullptr;
	_inPool += 1;
	_outPool -= 1;
	return true;
}

vector<unsigned char> RSAKeyManager::ExtractPublicKey(EVP_PKEY* key) {
	std::vector<unsigned char> publicKey;

	if (!key)
		return publicKey;

	//길이 추출, 및 vector에 길이 설정
	int len = i2d_PUBKEY(key, nullptr);
	if (len <= 0)
		return publicKey;
	publicKey.resize(len);

	//vector의 데이터 공간에 public key를 삽입
	unsigned char* ptr = publicKey.data();
	if (i2d_PUBKEY(key, &ptr) <= 0)
		publicKey.clear();

	return publicKey;
}

vector<unsigned char> RSAKeyManager::Decrypt(EVP_PKEY* privateKey, const vector<unsigned char>& encrypted) {
	vector<unsigned char> decrypted;
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
	if (!ctx) {
		cout << "EVP_PKEY_CTX_new failed" << endl;
		return {};
	}

	if (EVP_PKEY_decrypt_init(ctx) <= 0) {
		cout << "EVP_PKEY_decrypt_init failed" << endl;
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	// RSA_PADDING - 클라이언트가 어떤 방식으로 암호화했는지와 일치해야 함
	// RSA-OAEP-SHA256을 사용했다면 다음 줄을 추가
	if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
		cout << "Failed to set padding" << endl;
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	// 복호화 결과 길이 확인
	size_t outLen = 0;
	if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, encrypted.data(), encrypted.size()) <= 0) {
		cout << "EVP_PKEY_decrypt (length) failed" << endl;
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	decrypted.resize(outLen);

	if (EVP_PKEY_decrypt(ctx, decrypted.data(), &outLen, encrypted.data(), encrypted.size()) <= 0) {
		cout << "EVP_PKEY_decrypt failed" << endl;
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	decrypted.resize(outLen);
	EVP_PKEY_CTX_free(ctx);

	return decrypted;
}

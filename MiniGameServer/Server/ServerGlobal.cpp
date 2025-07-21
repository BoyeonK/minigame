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
	//����, ���ν����忡�� 1�� ���� �� ���̱� ������
	//������ �ȿ�����ŭ�� multi threadȯ���� ������� �ʰ� �ۼ���.
	for (int i = 0; i < 100; i++) {
		EVP_PKEY* pkey = nullptr;
		EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
		if (!ctx) {
			cout << i << "��° ctx ���� ����" << endl;
			continue;
		}

		if (EVP_PKEY_keygen_init(ctx) <= 0) {
			cout << i << "��° keygen init ����" << endl;
			EVP_PKEY_CTX_free(ctx);
			continue;
		}

		if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
			cout << i << "��° key size set ����" << endl;
			EVP_PKEY_CTX_free(ctx);
			continue;
		}

		if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
			cout << i << "��° pkey ���� ����" << endl;
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
	//TODO : ���ǰ� �ִ� ��� RSA Key�� ȸ���ϰ� ���� ����Ǿ�� ��.
	//���� _outPool�� 0���� Ȯ���ϴ� ������ �߰��� ����.

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
		cout << "ctx ���� ����" << endl;
		return nullptr;
	}

	if (EVP_PKEY_keygen_init(ctx) <= 0) {
		cout << "Keygen init ����" << endl;
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}

	if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
		cout << "Key size set ����" << endl;
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}

	if (EVP_PKEY_keygen(ctx, &key) <= 0) {
		cout << "Key ���� ����" << endl;
		EVP_PKEY_CTX_free(ctx);
		return nullptr;
	}

	EVP_PKEY_CTX_free(ctx);
	_outPool += 1;
	return key;
}

//�̷� �� �˾����� ó������ _keyQueue�� unique_ptr�� �ٷ�� ť�μ� ���� �� �׷���.
bool RSAKeyManager::ReturnKey(EVP_PKEY*& key) {
	if (!key) {
		cout << "??����ü �� �����Ѱ���??" << endl;
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

	//���� ����, �� vector�� ���� ����
	int len = i2d_PUBKEY(key, nullptr);
	if (len <= 0)
		return publicKey;
	publicKey.resize(len);

	//vector�� ������ ������ public key�� ����
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

	// RSA_PADDING - Ŭ���̾�Ʈ�� � ������� ��ȣȭ�ߴ����� ��ġ�ؾ� ��
	// RSA-OAEP-SHA256�� ����ߴٸ� ���� ���� �߰�
	if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
		cout << "Failed to set padding" << endl;
		EVP_PKEY_CTX_free(ctx);
		return {};
	}

	// ��ȣȭ ��� ���� Ȯ��
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

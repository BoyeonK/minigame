#include "pch.h"
#include "ServerPacketHandler.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"

shared_ptr<PlayerSession> PSfactory() {
	return make_shared<PlayerSession>();
}
/*
std::vector<unsigned char> encrypt(EVP_PKEY* pubKey, const std::string& message) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubKey, nullptr);
    if (!ctx) return {};

    if (EVP_PKEY_encrypt_init(ctx) <= 0) return {};
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) return {};

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (const unsigned char*)message.c_str(), message.length()) <= 0)
        return {};

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_encrypt(ctx, outbuf.data(), &outlen, (const unsigned char*)message.c_str(), message.length()) <= 0)
        return {};

    outbuf.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return outbuf;
}

std::string decrypt(EVP_PKEY* privKey, const std::vector<unsigned char>& ciphertext) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privKey, nullptr);
    if (!ctx) return "";

    if (EVP_PKEY_decrypt_init(ctx) <= 0) return "";
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) return "";

    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, ciphertext.data(), ciphertext.size()) <= 0)
        return "";

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_decrypt(ctx, outbuf.data(), &outlen, ciphertext.data(), ciphertext.size()) <= 0)
        return "";

    EVP_PKEY_CTX_free(ctx);
    return std::string(outbuf.begin(), outbuf.begin() + outlen);
}
*/
int main() {
	cout << "I'm Server" << endl;

	ServerPacketHandler::Init();

    /*
    std::string msg = "Hello RSA with OpenSSL 3.0!";
    std::vector<unsigned char> encrypted = encrypt(keypair, msg);

    if (encrypted.empty()) {
        std::cerr << "암호화 실패!" << std::endl;
        return 1;
    }

    std::string decrypted = decrypt(keypair, encrypted);
    std::cout << "복호화된 메시지: " << decrypted << std::endl;

    EVP_PKEY_free(keypair);
    */
	shared_ptr<ServerService> serverService = make_shared<ServerService>(
		make_shared<CPCore>(),
		NetAddress(L"0.0.0.0", 7777),
		PSfactory,
		100
	);

	serverService->StartAccept();

	GThreadManager->Launch([=]() {
		while (true) {
			ThreadManager::DoTimerQueueDistribution();
			this_thread::sleep_for(20ms);
		}
	});

	for (int i = 0; i < 4; i++) {
		GThreadManager->Launch([=]() {
			while (true) {
				LEndTickCount = ::GetTickCount64() + 64;
				ThreadManager::DoGlobalQueueWork();
				serverService->GetCPCoreRef()->Dispatch(10);
			}
		});
	}

	GThreadManager->Join();

}
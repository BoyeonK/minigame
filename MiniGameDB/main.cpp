#include "pch.h"
#include "GlobalVariables.h"
#include "DBServiceImpl.h"
#include <grpcpp/grpcpp.h>
#include <fstream>
#include <sstream>

string ReadFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    shared_ptr<GreeterServiceImpl> DBService = make_shared<GreeterServiceImpl>();
    string server_address("0.0.0.0:50051");

    string server_key = ReadFile("server.key");
    string server_cert = ReadFile("server.crt");

    if (server_key.empty() || server_cert.empty()) {
        cerr << "Error: server.key or server.crt not found!" << endl;
        return -1;
    }

    grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
    pkcp.private_key = server_key;
    pkcp.cert_chain = server_cert;

    grpc::SslServerCredentialsOptions ssl_opts;
    ssl_opts.pem_key_cert_pairs.push_back(pkcp);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::SslServerCredentials(ssl_opts));
    builder.RegisterService(DBService.get()); // 비동기 서비스 등록

    unique_ptr<grpc::ServerCompletionQueue> completionQueue(builder.AddCompletionQueue());
    DBService->setCompletionQueue(move(completionQueue));
    unique_ptr<grpc::Server> server(builder.BuildAndStart());
    cout << "Async Server listening" << endl;

    ReadyForCall(DBService.get(), DBService->getPCompletionQueue());

    for (int i = 0; i < 4; i++) {
        GThreadManager->Launch([=]() {
            while (true) {
                DBService->HandleRpcs();
            }
        });
    }

    server->Wait();

    DBService->getPCompletionQueue()->Shutdown();
    
    GThreadManager->Join();

    return 0;
}
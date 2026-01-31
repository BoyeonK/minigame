#include "pch.h"
#include "GlobalVariables.h"
#include "DBServiceImpl.h"

int main() {
    shared_ptr<GreeterServiceImpl> DBService = make_shared<GreeterServiceImpl>();
    string server_address("127.0.0.1:50051");
    grpc::ServerBuilder builder;

    grpc::SslServerCredentialsOptions ssl_opts;
    ssl_opts.pem_key_cert_pairs.push_back({
    R"(-----BEGIN RSA PRIVATE KEY-----
...
-----END RSA PRIVATE KEY-----)",
R"(-----BEGIN CERTIFICATE-----
...
-----END CERTIFICATE-----)"
        });

    builder.AddListeningPort(
        "127.0.0.1:50051",
        grpc::SslServerCredentials(ssl_opts)
    );

    builder.RegisterService(DBService.get());

    unique_ptr<grpc::ServerCompletionQueue> completionQueue(builder.AddCompletionQueue());
    DBService->setCompletionQueue(move(completionQueue));
    unique_ptr<grpc::Server> server(builder.BuildAndStart());
    cout << "Async Server listening on " << string(server_address.begin(), server_address.end()) << endl;

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
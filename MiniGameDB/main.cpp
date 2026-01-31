#include "pch.h"
#include "GlobalVariables.h"
#include "DBServiceImpl.h"

int main() {
    shared_ptr<GreeterServiceImpl> DBService = make_shared<GreeterServiceImpl>();
    string server_address("127.0.0.1:50051");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(DBService.get()); // 비동기 서비스 등록

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
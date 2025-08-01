#include "pch.h"
#include "GlobalVariables.h"
#include "DBServiceImpl.h"

int main() {
    GreeterServiceImpl DBService;

    string server_address("0.0.0.0:50051");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&DBService); // 비동기 서비스 등록

    unique_ptr<grpc::ServerCompletionQueue> completionQueue(builder.AddCompletionQueue());
    DBService.setCompletionQueue(move(completionQueue));
    unique_ptr<grpc::Server> server(builder.BuildAndStart());
    cout << "Async Server listening on " << string(server_address.begin(), server_address.end()) << endl;

    ReadyForCall(&DBService, DBService.getPCompletionQueue());

    server->Wait();

    //DBService.getPCompletionQueue()->Shutdown();
    /*
    for (auto& t : threads) {
        t.join();
    }
    */

    return 0;
}
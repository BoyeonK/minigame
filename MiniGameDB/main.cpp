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

    // 초기 요청 대기를 위해 워커 스레드 수만큼 CallData 객체 등록
    /*
    const int num_threads = std::thread::hardware_concurrency();
    for (int i = 0; i < num_threads; ++i) {
        new CallData(this, cq_.get());
    }
    */

    // CPU 코어 수만큼 워커 스레드 생성 및 실행
    /*
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(&GreeterServiceImpl::HandleRpcs, this);
    }
    */

    server->Wait();

    //completionQueue->Shutdown();
    /*
    for (auto& t : threads) {
        t.join();
    }
    */

    return 0;
}
#include "pch.h"
#include "GlobalVariables.h"
#include "DBServiceImpl.h"

int main() {
    GreeterServiceImpl DBService;

    string server_address("0.0.0.0:50051");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&DBService); // �񵿱� ���� ���

    unique_ptr<grpc::ServerCompletionQueue> completionQueue(builder.AddCompletionQueue());
    DBService.setCompletionQueue(move(completionQueue));
    unique_ptr<grpc::Server> server(builder.BuildAndStart());
    cout << "Async Server listening on " << string(server_address.begin(), server_address.end()) << endl;

    // �ʱ� ��û ��⸦ ���� ��Ŀ ������ ����ŭ CallData ��ü ���
    /*
    const int num_threads = std::thread::hardware_concurrency();
    for (int i = 0; i < num_threads; ++i) {
        new CallData(this, cq_.get());
    }
    */

    // CPU �ھ� ����ŭ ��Ŀ ������ ���� �� ����
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
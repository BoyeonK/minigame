#pragma once

#include <grpcpp/grpcpp.h>      // gRPC 라이브러리
#include "S2D_Protocol.grpc.pb.h" // protoc 컴파일로 생성된 헤더 파일

class GreeterClient {
private:
    unique_ptr<S2D_Protocol::Greeter::Stub> stub_;
    unique_ptr<grpc::CompletionQueue> cq_;
    thread completion_thread_;

    // AsyncClientCall: 각 RPC 호출의 상태를 관리하는 객체
    struct AsyncClientCall {
        S2D_Protocol::HelloReply reply;
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::HelloReply>> response_reader;
    };

    // CompletionQueue에서 이벤트를 폴링하고 처리하는 스레드
    void AsyncCompleteRpc() {
        void* tag;
        bool ok;
        while (cq_->Next(&tag, &ok)) {
            AsyncClientCall* call = static_cast<AsyncClientCall*>(tag);

            if (ok && call->status.ok()) {
                cout << "Client: Received reply from server: "
                    << string(call->reply.message().begin(), call->reply.message().end()) << endl;
            }
            else {
                cerr << "Client: RPC failed with code " << call->status.error_code() << " and message "
                    << string(call->status.error_message().begin(), call->status.error_message().end()) << endl;
            }
            delete call; // RPC가 완료되면 객체 해제
        }
    }

public:
    GreeterClient(shared_ptr<grpc::Channel> channel)
        : stub_(S2D_Protocol::Greeter::NewStub(channel)), cq_(make_unique<grpc::CompletionQueue>()) {
        completion_thread_ = thread(&GreeterClient::AsyncCompleteRpc, this);
    }

    ~GreeterClient() {
        cq_->Shutdown();
        if (completion_thread_.joinable()) {
            completion_thread_.join();
        }
    }

    void SayHelloAsync(const string& user) {
        AsyncClientCall* call = new AsyncClientCall();

        // 1. 요청 메시지 객체를 먼저 생성하고 데이터를 설정
        S2D_Protocol::HelloRequest request;
        request.set_name(user);

        // 2. 준비된 요청 객체를 PrepareAsyncSayHello 함수에 전달
        call->response_reader = stub_->PrepareAsyncSayHello(&call->context, request, cq_.get());
        call->response_reader->StartCall();

        // 3. 응답을 기다리며 CompletionQueue에 태그를 등록
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    }
};

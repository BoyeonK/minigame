#pragma once

#include <grpcpp/grpcpp.h>      // gRPC ���̺귯��
#include "S2D_Protocol.grpc.pb.h" // protoc �����Ϸ� ������ ��� ����

class GreeterClient {
private:
    unique_ptr<S2D_Protocol::Greeter::Stub> stub_;
    unique_ptr<grpc::CompletionQueue> cq_;
    thread completion_thread_;

    // AsyncClientCall: �� RPC ȣ���� ���¸� �����ϴ� ��ü
    struct AsyncClientCall {
        S2D_Protocol::HelloReply reply;
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::HelloReply>> response_reader;
    };

    // CompletionQueue���� �̺�Ʈ�� �����ϰ� ó���ϴ� ������
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
            delete call; // RPC�� �Ϸ�Ǹ� ��ü ����
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

        // 1. ��û �޽��� ��ü�� ���� �����ϰ� �����͸� ����
        S2D_Protocol::HelloRequest request;
        request.set_name(user);

        // 2. �غ�� ��û ��ü�� PrepareAsyncSayHello �Լ��� ����
        call->response_reader = stub_->PrepareAsyncSayHello(&call->context, request, cq_.get());
        call->response_reader->StartCall();

        // 3. ������ ��ٸ��� CompletionQueue�� �±׸� ���
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
    }
};

#pragma once

#include <grpcpp/grpcpp.h>      // gRPC ���̺귯��
#include "S2D_Protocol.grpc.pb.h" // protoc �����Ϸ� ������ ��� ����

class GreeterClient {
public:
    // ������: ���� �ּҸ� �޾� ä�ΰ� ������ �ʱ�ȭ
    GreeterClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(S2D_Protocol::Greeter::NewStub(channel)) {}

    // SayHello RPC�� ȣ���ϴ� �޼���
    std::string SayHello(const std::string& user) {
        S2D_Protocol::HelloRequest request; // ��û �޽��� ����
        request.set_name(user);              // �̸� �ʵ� ����

        S2D_Protocol::HelloReply reply;    // ���� �޽����� ���� ����
        grpc::ClientContext context;         // RPC ȣ���� ���ؽ�Ʈ

        // ���� RPC ȣ��
        grpc::Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok()) {
            // ȣ�� ���� �� ���� �޽��� ��ȯ
            std::cout << "Client: RPC successful! Server replied: " << string(reply.message().begin(), reply.message().end()) << std::endl;
            return reply.message();
        }
        else {
            // ȣ�� ���� �� ���� ���� ���
            std::cerr << "Client: RPC failed. Code: " << status.error_code()
                << ", Message: " << std::string(status.error_message().begin(), status.error_message().end()) << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<S2D_Protocol::Greeter::Stub> stub_; // ���� ���񽺿� ����� ����
};

class GreeterServiceImpl final : public S2D_Protocol::Greeter::AsyncService {
public:
    GreeterServiceImpl() {}

    // �� Ŭ�������� RPC ȣ���� ���ؽ�Ʈ�� �����մϴ�.
    // �� RPC ȣ�⸶�� �ϳ��� CallData ��ü�� �����Ǿ� �ش� ȣ���� ���¸� �����մϴ�.
    class CallData {
    public:
        CallData(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
            // �ʱ� ����: ���ο� ��û�� ��ٸ��ϴ�.
            Proceed();
        }

        void Proceed() {
            if (status_ == CREATE) {
                // �� CallData ��ü�� ����Ͽ� SayHello RPC�� ��ٸ����� ť�� ����մϴ�.
                // �� �������� RPC�� ���� ���۵��� �ʾҽ��ϴ�.
                status_ = PROCESS;
                service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
            }
            else if (status_ == PROCESS) {
                // RPC ��û�� �޾ҽ��ϴ�. ���� ���� ��û�� ���� ���ο� CallData ��ü�� �����մϴ�.
                new CallData(service_, cq_); // ���� ��û�� ���� ���ο� ��ü ����

                // ���� ��û ó��: HelloReply �޽��� ����
                std::string name = request_.name();
                std::string message = "Hello, " + name + "! (Async)";
                reply_.set_message(message);

                std::wcout << L"Server: Received request for name: " << std::wstring(name.begin(), name.end()) << std::endl;
                std::wcout << L"Server: Sending reply: " << std::wstring(message.begin(), message.end()) << std::endl;

                // ������ �����ϴ�. send_message�� �񵿱� �۾��Դϴ�.
                status_ = FINISH;
                responder_.Finish(reply_, grpc::Status::OK, this);
            }
            else {
                // status_ == FINISH
                // RPC�� �Ϸ�Ǿ����ϴ�. �� CallData ��ü�� �����մϴ�.
                delete this;
            }
        }

    private:
        S2D_Protocol::Greeter::AsyncService* service_;    // ���� ������
        grpc::ServerCompletionQueue* cq_;                  // CompletionQueue ������
        grpc::ServerContext ctx_;                          // RPC ���ؽ�Ʈ (��Ÿ������ ��)

        // ��û/���� �޽���
        S2D_Protocol::HelloRequest request_;
        S2D_Protocol::HelloReply reply_;

        // ���� ������ (RPC ���� ����)
        grpc::ServerAsyncResponseWriter<S2D_Protocol::HelloReply> responder_;

        enum CallStatus { CREATE, PROCESS, FINISH }; // RPC�� ���� ����
        CallStatus status_; // ���� ����
    };


    // ���� ������ �Լ�
    void HandleRpcs() {
        // CompletionQueue���� �̺�Ʈ�� ���⸦ ��ٸ��ϴ�.
        // �� ������� ť�� ��ȯ�ϴ� ��� �̺�Ʈ�� ó���մϴ�.
        // ���� ��Ŀ �����尡 �� �Լ��� ���ÿ� ������ �� �ֽ��ϴ�.
        void* tag;       // �̺�Ʈ�� ����� �±� (���⼭�� CallData ��ü)
        bool ok;         // �̺�Ʈ�� ���������� ����

        while (true) {
            // ť���� ���� �̺�Ʈ�� �����ɴϴ�. ���ŷ ȣ���Դϴ�.
            if (!cq_->Next(&tag, &ok)) {
                // cq_�� Shutdown�Ǹ� false�� ��ȯ�ϰ�, �ݺ����� Ż���մϴ�.
                break;
            }

            // tag�� CallData ��ü �����ͷ� ��ȯ�մϴ�.
            GreeterServiceImpl::CallData* call_data = static_cast<GreeterServiceImpl::CallData*>(tag);
            // tag�� CallData ��ü �������Դϴ�.
            // ok�� �̺�Ʈ�� ���������� �Ϸ�Ǿ����� ���θ� ��Ÿ���ϴ�.
            static_cast<CallData*>(tag)->Proceed();
        }
    }

private:
    unique_ptr<grpc::ServerCompletionQueue> cq_; // Completion Queue
};

void RunAsyncServer() {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service); // �񵿱� ���� ���

    // CompletionQueue ���� �� ������ �߰�
    unique_ptr<grpc::ServerCompletionQueue> cq(builder.AddCompletionQueue());
    //service.set_cq(cq.get()); // ���� ����ü�� CompletionQueue ���� (���ǻ�)

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::wcout << L"Async Server listening on " << std::wstring(server_address.begin(), server_address.end()) << std::endl;

    // �ʱ� ��û ��⸦ ���� CallData ��ü ����
    // �ּ��� �ϳ��� CallData ��ü�� �־�� ��û�� ���� �� �ֽ��ϴ�.
    new GreeterServiceImpl::CallData(&service, cq.get());

    // RPC ó�� ������ ���� (���� ������ ��� ����)
    // �Ϲ������� CPU �ھ� ���� ����Ͽ� ��Ŀ �����带 �����մϴ�.
    std::vector<std::thread> threads;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) { // ����: �ھ� ����ŭ ������
        threads.emplace_back([&]() {
            service.HandleRpcs(); // �� �����尡 ť���� �̺�Ʈ�� ������ ó��
            });
    }

    // ������ ����� ������ ���
    server.get()->Wait(); // unique_ptr���� raw pointer ���

    // ���� ���� �� ť�� ������ ����
    cq->Shutdown();
    for (auto& t : threads) {
        t.join();
    }
}
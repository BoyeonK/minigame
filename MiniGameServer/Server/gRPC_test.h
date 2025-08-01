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
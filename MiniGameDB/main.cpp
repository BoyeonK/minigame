#include "GlobalVariables.h"

#include <grpcpp/grpcpp.h>
#include "S2D_Protocol.grpc.pb.h"

using namespace std;

class GreeterServiceImpl final : public S2D_Protocol::Greeter::Service {
public:
    // SayHello RPC �޼��� ����
    grpc::Status SayHello(grpc::ServerContext* context, const S2D_Protocol::HelloRequest* request, S2D_Protocol::HelloReply* reply) override {
        // Ŭ���̾�Ʈ�κ��� ���� name ���� ������
        string name = request->name();

        // ���� �޽��� ����
        string message = "Hello, " + name + "!";
        reply->set_message(message);

        // �ֿܼ� ���� ��û�� ���� ������ ��� (������)
        cout << "Server: Received request for name: " << string(name.begin(), name.end()) << endl;
        cout << "Server: Sending reply: " << string(message.begin(), message.end()) << endl;

        return grpc::Status::OK; // RPC ȣ���� ���������� ��Ÿ��
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051"); // ��� IP���� 50051 ��Ʈ�� ����
    GreeterServiceImpl service; // ���� ����ü �ν��Ͻ�

    grpc::ServerBuilder builder; // gRPC ���� ����
    // ���� �ּҿ� ���� ��� ���� (���⼭�� ������ ������� ���� Insecure ���)
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // ������ ���� ���
    builder.RegisterService(&service);

    // ���� ���� �� ����
    unique_ptr<grpc::Server> server(builder.BuildAndStart());
    cout << "Server listening on " << string(server_address.begin(), server_address.end()) << endl;

    // ������ ����� ������ ��� (�� �Լ��� ��ȯ�Ǹ� ������ ����˴ϴ�)
    server->Wait();
}

int main() {

    RunServer();
    return 0;
}
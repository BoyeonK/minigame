#include "GlobalVariables.h"

#include <grpcpp/grpcpp.h>
#include "S2D_Protocol.grpc.pb.h"

using namespace std;

class GreeterServiceImpl final : public S2D_Protocol::Greeter::Service {
public:
    // SayHello RPC 메서드 구현
    grpc::Status SayHello(grpc::ServerContext* context, const S2D_Protocol::HelloRequest* request, S2D_Protocol::HelloReply* reply) override {
        // 클라이언트로부터 받은 name 값을 가져옴
        string name = request->name();

        // 응답 메시지 생성
        string message = "Hello, " + name + "!";
        reply->set_message(message);

        // 콘솔에 받은 요청과 보낼 응답을 출력 (디버깅용)
        cout << "Server: Received request for name: " << string(name.begin(), name.end()) << endl;
        cout << "Server: Sending reply: " << string(message.begin(), message.end()) << endl;

        return grpc::Status::OK; // RPC 호출이 성공했음을 나타냄
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051"); // 모든 IP에서 50051 포트로 수신
    GreeterServiceImpl service; // 서비스 구현체 인스턴스

    grpc::ServerBuilder builder; // gRPC 서버 빌더
    // 서버 주소와 인증 방식 지정 (여기서는 보안이 적용되지 않은 Insecure 사용)
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // 구현한 서비스 등록
    builder.RegisterService(&service);

    // 서버 생성 및 시작
    unique_ptr<grpc::Server> server(builder.BuildAndStart());
    cout << "Server listening on " << string(server_address.begin(), server_address.end()) << endl;

    // 서버가 종료될 때까지 대기 (이 함수가 반환되면 서버는 종료됩니다)
    server->Wait();
}

int main() {

    RunServer();
    return 0;
}
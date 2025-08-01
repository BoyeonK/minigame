#pragma once

#include <grpcpp/grpcpp.h>      // gRPC 라이브러리
#include "S2D_Protocol.grpc.pb.h" // protoc 컴파일로 생성된 헤더 파일

class GreeterClient {
public:
    // 생성자: 서버 주소를 받아 채널과 스텁을 초기화
    GreeterClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(S2D_Protocol::Greeter::NewStub(channel)) {}

    // SayHello RPC를 호출하는 메서드
    std::string SayHello(const std::string& user) {
        S2D_Protocol::HelloRequest request; // 요청 메시지 생성
        request.set_name(user);              // 이름 필드 설정

        S2D_Protocol::HelloReply reply;    // 응답 메시지를 받을 변수
        grpc::ClientContext context;         // RPC 호출의 컨텍스트

        // 실제 RPC 호출
        grpc::Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok()) {
            // 호출 성공 시 응답 메시지 반환
            std::cout << "Client: RPC successful! Server replied: " << string(reply.message().begin(), reply.message().end()) << std::endl;
            return reply.message();
        }
        else {
            // 호출 실패 시 오류 정보 출력
            std::cerr << "Client: RPC failed. Code: " << status.error_code()
                << ", Message: " << std::string(status.error_message().begin(), status.error_message().end()) << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<S2D_Protocol::Greeter::Stub> stub_; // 서버 서비스와 통신할 스텁
};
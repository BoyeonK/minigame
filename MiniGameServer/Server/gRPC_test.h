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

class GreeterServiceImpl final : public S2D_Protocol::Greeter::AsyncService {
public:
    GreeterServiceImpl() {}

    // 이 클래스에서 RPC 호출의 컨텍스트를 관리합니다.
    // 각 RPC 호출마다 하나의 CallData 객체가 생성되어 해당 호출의 상태를 추적합니다.
    class CallData {
    public:
        CallData(S2D_Protocol::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
            // 초기 상태: 새로운 요청을 기다립니다.
            Proceed();
        }

        void Proceed() {
            if (status_ == CREATE) {
                // 이 CallData 객체를 사용하여 SayHello RPC를 기다리도록 큐에 등록합니다.
                // 이 시점에서 RPC는 아직 시작되지 않았습니다.
                status_ = PROCESS;
                service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
            }
            else if (status_ == PROCESS) {
                // RPC 요청을 받았습니다. 이제 다음 요청을 위해 새로운 CallData 객체를 생성합니다.
                new CallData(service_, cq_); // 다음 요청을 위한 새로운 객체 생성

                // 받은 요청 처리: HelloReply 메시지 생성
                std::string name = request_.name();
                std::string message = "Hello, " + name + "! (Async)";
                reply_.set_message(message);

                std::wcout << L"Server: Received request for name: " << std::wstring(name.begin(), name.end()) << std::endl;
                std::wcout << L"Server: Sending reply: " << std::wstring(message.begin(), message.end()) << std::endl;

                // 응답을 보냅니다. send_message는 비동기 작업입니다.
                status_ = FINISH;
                responder_.Finish(reply_, grpc::Status::OK, this);
            }
            else {
                // status_ == FINISH
                // RPC가 완료되었습니다. 이 CallData 객체를 삭제합니다.
                delete this;
            }
        }

    private:
        S2D_Protocol::Greeter::AsyncService* service_;    // 서비스 포인터
        grpc::ServerCompletionQueue* cq_;                  // CompletionQueue 포인터
        grpc::ServerContext ctx_;                          // RPC 컨텍스트 (메타데이터 등)

        // 요청/응답 메시지
        S2D_Protocol::HelloRequest request_;
        S2D_Protocol::HelloReply reply_;

        // 서버 응답자 (RPC 상태 관리)
        grpc::ServerAsyncResponseWriter<S2D_Protocol::HelloReply> responder_;

        enum CallStatus { CREATE, PROCESS, FINISH }; // RPC의 현재 상태
        CallStatus status_; // 현재 상태
    };


    // 서버 스레드 함수
    void HandleRpcs() {
        // CompletionQueue에서 이벤트가 오기를 기다립니다.
        // 이 스레드는 큐가 반환하는 모든 이벤트를 처리합니다.
        // 여러 워커 스레드가 이 함수를 동시에 실행할 수 있습니다.
        void* tag;       // 이벤트와 연결된 태그 (여기서는 CallData 객체)
        bool ok;         // 이벤트가 성공적인지 여부

        while (true) {
            // 큐에서 다음 이벤트를 가져옵니다. 블로킹 호출입니다.
            if (!cq_->Next(&tag, &ok)) {
                // cq_가 Shutdown되면 false를 반환하고, 반복문을 탈출합니다.
                break;
            }

            // tag를 CallData 객체 포인터로 변환합니다.
            GreeterServiceImpl::CallData* call_data = static_cast<GreeterServiceImpl::CallData*>(tag);
            // tag는 CallData 객체 포인터입니다.
            // ok는 이벤트가 성공적으로 완료되었는지 여부를 나타냅니다.
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
    builder.RegisterService(&service); // 비동기 서비스 등록

    // CompletionQueue 생성 및 빌더에 추가
    unique_ptr<grpc::ServerCompletionQueue> cq(builder.AddCompletionQueue());
    //service.set_cq(cq.get()); // 서비스 구현체에 CompletionQueue 설정 (편의상)

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::wcout << L"Async Server listening on " << std::wstring(server_address.begin(), server_address.end()) << std::endl;

    // 초기 요청 대기를 위한 CallData 객체 생성
    // 최소한 하나의 CallData 객체가 있어야 요청을 받을 수 있습니다.
    new GreeterServiceImpl::CallData(&service, cq.get());

    // RPC 처리 스레드 실행 (여러 스레드 사용 가능)
    // 일반적으로 CPU 코어 수에 비례하여 워커 스레드를 생성합니다.
    std::vector<std::thread> threads;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) { // 예시: 코어 수만큼 스레드
        threads.emplace_back([&]() {
            service.HandleRpcs(); // 각 스레드가 큐에서 이벤트를 가져와 처리
            });
    }

    // 서버가 종료될 때까지 대기
    server.get()->Wait(); // unique_ptr에서 raw pointer 얻기

    // 서버 종료 시 큐와 스레드 정리
    cq->Shutdown();
    for (auto& t : threads) {
        t.join();
    }
}
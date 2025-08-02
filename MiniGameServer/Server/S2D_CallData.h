#pragma once

class S2D_CallData {
public:
	virtual ~S2D_CallData() { }

	//gRPC call�� �������� ��� �۵��� �Լ�
	virtual void OnSucceed() = 0;
	//�������� ���
	virtual void OnFailed() = 0;
	virtual void ReturnToPool() = 0;

	grpc::ClientContext context;
	grpc::Status status;
};

#ifdef _DEBUG
class HelloCall final : public S2D_CallData {
public:
	~HelloCall() { }

	void OnSucceed() override;
	void OnFailed() override;
	void ReturnToPool() {
		objectPool<HelloCall>::dealloc(this);
	}

	S2D_Protocol::HelloReply reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::HelloReply>> response_reader;
};
#endif

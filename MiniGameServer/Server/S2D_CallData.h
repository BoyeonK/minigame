#pragma once
#include "PlayerSession.h"

class S2D_CallData {
public:
	virtual ~S2D_CallData() { }

	//gRPC call이 성공했을 경우 작동할 함수
	virtual void OnSucceed() = 0;
	//실패했을 경우
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

class SLoginCall final : public S2D_CallData {
public:
	SLoginCall(weak_ptr<PBSession> sessionRef) : _clientSessionRef(sessionRef) { }
	~SLoginCall() { }

	void OnSucceed() override;
	void OnFailed() override;

	void CorrectI(int32_t dbid);
	void IncorrectI(bool incorrect_id);

	void ReturnToPool() {
		objectPool<SLoginCall>::dealloc(this);
	}

	S2D_Protocol::D2S_Login reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2S_Login>> response_reader;

private:
	weak_ptr<PBSession> _clientSessionRef;
};

class SCreateAccountCall final : public S2D_CallData {
public:
	SCreateAccountCall(weak_ptr<PBSession> sessionRef) : _clientSessionRef(sessionRef) { }
	~SCreateAccountCall() { }

	void OnSucceed() override;
	void OnFailed() override;
	
	void CreateComplete();
	void CreateFailed();

	void ReturnToPool() {
		objectPool<SCreateAccountCall>::dealloc(this);
	}

	S2D_Protocol::D2S_CreateAccount reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2S_CreateAccount>> response_reader;

private:
	weak_ptr<PBSession> _clientSessionRef;
};

class SRenewElosCall final : public S2D_CallData {
public:
	SRenewElosCall(weak_ptr<PlayerSession> playerSessionRef) : _clientSessionRef(playerSessionRef) {}
	~SRenewElosCall() {}
	
	void StartCall();
	void OnSucceed() override;
	void OnFailed() override;

	void ReturnToPool() {
		objectPool<SRenewElosCall>::dealloc(this);
	}

	S2D_Protocol::D2S_RenewElos reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2S_RenewElos>> response_reader;

private:
	weak_ptr<PlayerSession> _clientSessionRef;
};

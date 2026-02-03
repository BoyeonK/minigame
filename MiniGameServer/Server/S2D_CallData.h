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

class SPlayerInformationCall final : public S2D_CallData {
public:
	SPlayerInformationCall(weak_ptr<PlayerSession> playerSessionRef) : _clientSessionRef(playerSessionRef) {}
	~SPlayerInformationCall() {}

	void StartCall();
	void OnSucceed() override;
	void OnFailed() override;

	void ReturnToPool() {
		objectPool<SPlayerInformationCall>::dealloc(this);
	}

	S2D_Protocol::D2C_ResponsePlayerInfomation reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2C_ResponsePlayerInfomation>> response_reader;

private:
	weak_ptr<PlayerSession> _clientSessionRef;
};

class SUpdateEloCall final : public S2D_CallData {
public:
	SUpdateEloCall(int32_t gameId, int32_t elo) : _gameId(gameId), _elo(elo) {}
	~SUpdateEloCall() {}

	void StartCall();
	void OnSucceed() override;
	void OnFailed() override;

	void ReturnToPool() {
		objectPool<SUpdateEloCall>::dealloc(this);
	}

	S2D_Protocol::D2S_ResponseUpdateElo reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2S_ResponseUpdateElo>> response_reader;

private:
	int32_t _gameId = 0;
	int32_t _elo = 0;
};

class SUpdatePersonalRecordCall final : public S2D_CallData {
public:
	SUpdatePersonalRecordCall(weak_ptr<PlayerSession> playerSessionRef, int32_t gameId, int32_t score) : _clientSessionRef(playerSessionRef), _gameId(gameId), _score(score) {}
	~SUpdatePersonalRecordCall() {}

	void StartCall();
	void OnSucceed() override;
	void OnFailed() override;

	void ReturnToPool() {
		objectPool<SUpdatePersonalRecordCall>::dealloc(this);
	}
	
	S2D_Protocol::D2S_ResponseUpdatePersonalRecord reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2S_ResponseUpdatePersonalRecord>> response_reader;

private:
	weak_ptr<PlayerSession> _clientSessionRef;
	int32_t _gameId = 0;
	int32_t _score = 0;
};

class SPublicRecordCall final : public S2D_CallData {
public:
	SPublicRecordCall(int32_t gameId) : _gameId(gameId) {}
	~SPublicRecordCall() {}

	void StartCall();
	void OnSucceed() override;
	void OnFailed() override;

	void ReturnToPool() {
		objectPool<SPublicRecordCall>::dealloc(this);
	}

	S2D_Protocol::D2S_ResponsePublicRecord reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2S_ResponsePublicRecord>> response_reader;

private:
	int32_t _gameId;
};

class SUpdatePublicRecordCall final : public S2D_CallData {
public:
	SUpdatePublicRecordCall(int32_t gameId) : _gameId(gameId) {}
	~SUpdatePublicRecordCall() {}

	void StartCall();
	void OnSucceed() override;
	void OnFailed() override;

	void ReturnToPool() {
		objectPool<SUpdatePublicRecordCall>::dealloc(this);
	}

	S2D_Protocol::D2C_ResponseUpdatePublicRecord reply;
	std::unique_ptr<grpc::ClientAsyncResponseReader<S2D_Protocol::D2C_ResponseUpdatePublicRecord>> response_reader;

private:
	int32_t _gameId;
};
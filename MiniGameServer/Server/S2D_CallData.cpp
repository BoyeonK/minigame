#include "pch.h"
#include "S2D_CallData.h"
#include "S2CPacketMaker.h"
#include "S2CPacketHandler.h"
#include "S2C_Protocol.pb.h"

#ifdef _DEBUG
void HelloCall::OnSucceed() {
	cout << "Client: Received reply from server: "
		<< string(this->reply.message().begin(), this->reply.message().end()) << endl;
}

void HelloCall::OnFailed() {
	cerr << "Client: RPC failed with code " << this->status.error_code() << " and message "
		<< string(this->status.error_message().begin(), this->status.error_message().end()) << endl;
}
#endif

void SLoginCall::OnSucceed() {
	cout << "DB������ �α��� ��û�� ���� ������ ����" << endl;

	if (this->reply.has_dbid()) {
		CorrectI(this->reply.dbid());
	}
	else {
		cout << "���� ���̵�" << endl;
		IncorrectI(this->reply.incorrect_id());
	}
	//TODO: 0���������, S_Login ��Ŷ�� err�� ��Ƽ� Ŭ�� ����.
	//0 �ܸ̿� ���� ���, �ش� dbid�� Ŭ���̾�Ʈ���� '��ȣȭ�ؼ�' ����
}

void SLoginCall::OnFailed() {
	//TODO: ���� �ؾߵɰ� ������ ���� ������ �ȳ���
}

void SLoginCall::CorrectI(int32_t dbid) {
	S2C_Protocol::S_Login pkt = S2CPacketMaker::MakeSLogin(dbid);
	shared_ptr<PlayerSession> sessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	//dbid = 0�ΰ�� (�α��� ����)
	//S_Login��Ŷ�� �׳� ����.
	if (dbid == 0) {
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
		sessionRef->Send(sendBufferRef);
	}
	//dbid = 0�� �ƴ� ��� (�α��� ����)
	//S_Login��Ŷ�� session�� ��ȣȭ Ű�� ��ȣȭ�Ͽ� ����.
	else {
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt, sessionRef->GetAESKey());
		sessionRef->Send(sendBufferRef);
	}
}

void SLoginCall::IncorrectI(bool incorrect_id) {
	S2C_Protocol::S_Login pkt = S2CPacketMaker::MakeSLogin(incorrect_id);
	shared_ptr<PlayerSession> sessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());

	//�α��� ���� (���� ���̵�)
	shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
	sessionRef->Send(sendBufferRef);
}

void SCreateAccountCall::OnSucceed() {
	cout << "DB������ ���� ���� ��û�� ������ ����" << endl;
	//TODO: ���� ���� ����, ���п��θ� bool������ Ȯ���Ѵ�.
	//���Ŀ��� �� ����, ���п��θ� Ŭ���̾�Ʈ�� �뺸�� ���� ������.
}

void SCreateAccountCall::OnFailed() {
	//TODO: ���� �ؾߵɰ� ������ ���� ������ �ȳ���
}

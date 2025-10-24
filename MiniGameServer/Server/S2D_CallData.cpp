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
	shared_ptr<PlayerSession> playerSessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	//dbid = 0�ΰ�� (�α��� ����)
	//S_Login��Ŷ�� �׳� ����.
	if (dbid == 0) {
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBufferRef);
	}
	//dbid = 0�� �ƴ� ��� (�α��� ����)
	//S_Login��Ŷ�� session�� ��ȣȭ Ű�� ��ȣȭ�Ͽ� ����.
	else {
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt, playerSessionRef->GetAESKey());
		playerSessionRef->SetDbid(dbid);
		//�ش� session���� ó���� �ִ� msgId�� 10000���� ���� (��ǻ�, ���� ��� ��Ŷ�� ���� ��û�� �������� �ʰڴٴ� ��)
		playerSessionRef->SetSecureLevel(10000);
		playerSessionRef->Send(sendBufferRef);

		//��� �α����� ���ǿ� Elo�� �ֽ�ȭ
		//DBManager->S2D_RenewElos(playerSessionRef, dbid);

		//Elo�Ӹ��� �ƴ϶� ���� ������ ������
		DBManager->S2D_PlayerInfomation(playerSessionRef, dbid);
	}
}

void SLoginCall::IncorrectI(bool incorrect_id) {
	S2C_Protocol::S_Login pkt = S2CPacketMaker::MakeSLogin(incorrect_id);
	shared_ptr<PlayerSession> sessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	if (sessionRef == nullptr) {
		return;
	}

	//�α��� ���� (���� ���̵�)
	shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
	sessionRef->Send(sendBufferRef);
}

void SCreateAccountCall::OnSucceed() {
	cout << "DB������ ���� ���� ��û�� ������ ����" << endl;
	if (reply.success()) {
		CreateComplete();
		cout << "���� ���� ��û ����." << endl;
	}
	else {
		CreateFailed();
		cout << "���� ���� ��û ����." << endl;
	}
}

void SCreateAccountCall::OnFailed() {
	//TODO: ���� �ؾߵɰ� ������ ���� ������ �ȳ���
}

void SCreateAccountCall::CreateComplete() {
	S2C_Protocol::S_CreateAccount pkt;
	pkt.set_success(true);

	shared_ptr<PlayerSession> sessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	if (sessionRef == nullptr) {
		return;
	}
	shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
	sessionRef->Send(sendBufferRef);
}

void SCreateAccountCall::CreateFailed() {
	S2C_Protocol::S_CreateAccount pkt;
	pkt.set_success(false);
	pkt.set_err("Failed To create Account");

	shared_ptr<PlayerSession> sessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	if (sessionRef == nullptr) {
		return;
	}
	shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
	sessionRef->Send(sendBufferRef);
}
/*
void SRenewElosCall::OnSucceed() {
	shared_ptr<PlayerSession> sessionRef = _clientSessionRef.lock();
	if (sessionRef == nullptr) {
		return;
	}

	sessionRef->SetElos(reply.elo1(), reply.elo2(), reply.elo3());
	cout << "Elo ���� �Ϸ�" << endl;
}

void SRenewElosCall::OnFailed() {
	//TODO: ���� �ؾߵɰ� ������ ���� ������ �ȳ���
}
*/
void SPlayerInformationCall::OnSucceed() {
	shared_ptr<PlayerSession> playerSessionRef = _clientSessionRef.lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	playerSessionRef->SetPlayerId(reply.playerid());
	for (int i = 0; i < reply.elos_size(); i++) {
		playerSessionRef->SetElo(i, reply.elos(i));
	}
	for (int i = 0; i < reply.personalrecords_size(); i++) {
		playerSessionRef->SetPersonalRecord(i, reply.personalrecords(i));
	}
	cout << "Elo ���� �Ϸ�" << endl;
}

void SPlayerInformationCall::OnFailed() {

}

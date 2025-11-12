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
	cout << "DB서버에 로그인 요청에 대한 응답을 받음" << endl;

	if (this->reply.has_dbid()) {
		CorrectI(this->reply.dbid());
	}
	else {
		cout << "없는 아이디" << endl;
		IncorrectI(this->reply.incorrect_id());
	}
	//TODO: 0을받은경우, S_Login 패킷에 err를 담아서 클라에 전송.
	//0 이외를 받은 경우, 해당 dbid를 클라이언트에게 '암호화해서' 전송
}

void SLoginCall::OnFailed() {
	//TODO: 뭔가 해야될거 같은데 당장 생각이 안나네
}

void SLoginCall::CorrectI(int32_t dbid) {
	S2C_Protocol::S_Login pkt = S2CPacketMaker::MakeSLogin(dbid);
	shared_ptr<PlayerSession> playerSessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	//dbid = 0인경우 (로그인 실패)
	//S_Login패킷을 그냥 전송.
	if (dbid == 0) {
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBufferRef);
	}
	//dbid = 0이 아닌 경우 (로그인 성공)
	//S_Login패킷을 session의 암호화 키로 암호화하여 전송.
	else {
		shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt, playerSessionRef->GetAESKey());
		playerSessionRef->SetDbid(dbid);
		//해당 session에서 처리할 최대 msgId를 10000으로 설정 (사실상, 이제 모든 패킷에 대한 요청을 거절하지 않겠다는 뜻)
		playerSessionRef->SetSecureLevel(10000);
		playerSessionRef->Send(sendBufferRef);

		//방금 로그인한 세션에 Elo를 최신화
		//DBManager->S2D_RenewElos(playerSessionRef, dbid);

		//Elo뿐만이 아니라 여러 정보를 가져옴
		DBManager->S2D_PlayerInfomation(playerSessionRef, dbid);
	}
}

void SLoginCall::IncorrectI(bool incorrect_id) {
	S2C_Protocol::S_Login pkt = S2CPacketMaker::MakeSLogin(incorrect_id);
	shared_ptr<PlayerSession> sessionRef = static_pointer_cast<PlayerSession>(_clientSessionRef.lock());
	if (sessionRef == nullptr) {
		return;
	}

	//로그인 실패 (없는 아이디)
	shared_ptr<SendBuffer> sendBufferRef = S2CPacketHandler::MakeSendBufferRef(pkt);
	sessionRef->Send(sendBufferRef);
}

void SCreateAccountCall::OnSucceed() {
	cout << "DB서버에 계정 생성 요청의 응답을 받음" << endl;
	if (reply.success()) {
		CreateComplete();
		cout << "계정 생성 요청 성공." << endl;
	}
	else {
		CreateFailed();
		cout << "계정 생성 요청 실패." << endl;
	}
}

void SCreateAccountCall::OnFailed() {
	//TODO: 뭔가 해야될거 같은데 당장 생각이 안나네
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

void SPlayerInformationCall::OnSucceed() {
	shared_ptr<PlayerSession> playerSessionRef = _clientSessionRef.lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	playerSessionRef->SetPlayerId(reply.playerid());
	for (int i = 0; i < reply.elos_size(); i++) {
		playerSessionRef->SetElo(i + 1, reply.elos(i));
	}
	for (int i = 0; i < reply.personalrecords_size(); i++) {
		playerSessionRef->SetPersonalRecord(i + 1, reply.personalrecords(i));
	}
	cout << "DB로부터 player의 정보를 가져와서 session에 채움" << endl;

	S2C_Protocol::S_ResponseMyRecords pkt = S2CPacketMaker::MakeSResponseMyRecords(playerSessionRef);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	playerSessionRef->Send(sendBuffer);
}

void SPlayerInformationCall::OnFailed() {

}

void SUpdateEloCall::OnSucceed() {
	shared_ptr<PlayerSession> playerSessionRef = _clientSessionRef.lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	if (!reply.success())
		return;

	if (_gameId == 0 || _elo == 0)
		return;

	playerSessionRef->SetElo(_gameId, _elo);
}

void SUpdateEloCall::OnFailed() {

}

void SUpdatePersonalRecordCall::OnSucceed() {
	shared_ptr<PlayerSession> playerSessionRef = _clientSessionRef.lock();
	if (PlayerSession::IsInvalidPlayerSession(playerSessionRef))
		return;

	if (!reply.success())
		return;

	if (_gameId == 0 || _score == 0)
		return;

	playerSessionRef->SetPersonalRecord(_gameId, _score);
}

void SUpdatePersonalRecordCall::OnFailed() {

}

void SPublicRecordCall::OnSucceed() {
	GGameManagers[_gameId]->SetPublicRecord(reply.playerid(), reply.publicrecord());
	cout << _gameId << "번 레코드 " << reply.playerid() << "의 점수 " << reply.publicrecord() << endl;
}

void SPublicRecordCall::OnFailed() {

}

void SUpdatePublicRecordCall::OnSucceed() {
#ifdef _DEBUG
	if (reply.success()) {
		cout << "갱신성공" << endl;
	}
	else {
		cout << "모종의 이유로 실패" << endl;
	}
#endif
}

void SUpdatePublicRecordCall::OnFailed() {

}

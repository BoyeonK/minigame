#include "pch.h"
#include "S2D_CallData.h"

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
	//TODO: 0을받은경우, S_Login 패킷에 err를 담아서 클라에 전송.
	//0 이외를 받은 경우, 해당 dbid를 클라이언트에게 '암호화해서' 전송
}

void SLoginCall::OnFailed() {
	//TODO: 뭔가 해야될거 같은데 당장 생각이 안나네
}

void SCreateAccountCall::OnSucceed() {
	//TODO: 계정 생성 성공, 실패여부를 bool값으로 확인한다.
	//추후에는 이 성공, 실패여부를 클라이언트에 통보할 일이 있을것.
}

void SCreateAccountCall::OnFailed() {
	//TODO: 뭔가 해야될거 같은데 당장 생각이 안나네
}

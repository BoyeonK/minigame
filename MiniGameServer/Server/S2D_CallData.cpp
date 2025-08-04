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
	//TODO: 0���������, S_Login ��Ŷ�� err�� ��Ƽ� Ŭ�� ����.
	//0 �ܸ̿� ���� ���, �ش� dbid�� Ŭ���̾�Ʈ���� '��ȣȭ�ؼ�' ����
}

void SLoginCall::OnFailed() {
	//TODO: ���� �ؾߵɰ� ������ ���� ������ �ȳ���
}

void SCreateAccountCall::OnSucceed() {
	//TODO: ���� ���� ����, ���п��θ� bool������ Ȯ���Ѵ�.
	//���Ŀ��� �� ����, ���п��θ� Ŭ���̾�Ʈ�� �뺸�� ���� ������.
}

void SCreateAccountCall::OnFailed() {
	//TODO: ���� �ؾߵɰ� ������ ���� ������ �ȳ���
}

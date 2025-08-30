#include "pch.h"
#include "GameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void PingPongGameRoom::Init(vector<WatingPlayerData> pdv) {
	//�� Session�� KeepAlive��Ŷ�� BroadCast.
	//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
	//��� Session���κ��� ������ ���� ���ߴٸ� 
		//������ Session�� �ٽ� ��⿭��...
		//�������� ���� Session�� ��ġ�� ��� or ������ ����.
	bool ready = true;
	vector<WatingPlayerData> rematchPd;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd._playerSessionRef.lock();

		if (playerSessionRef == nullptr) {
			ready = false;
			continue;
		}

		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(1);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
		rematchPd.push_back(pd);
	}

	if (ready) {
		DoTimerAsync(1000, &PingPongGameRoom::Init2, pdv);
	}
	else {

	}
}

void PingPongGameRoom::Init2(vector<WatingPlayerData> pdv) {

}

void PingPongGameRoom::ReturnToPool() {
	objectPool<PingPongGameRoom>::dealloc(this);
}

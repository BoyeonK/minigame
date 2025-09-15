#include "pch.h"
#include "GameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

GameRoom::GameState GameRoom::GetState() {
	return _state;
}

void PingPongGameRoom::Init(vector<WatingPlayerData> pdv) {
	bool ready = true;
	vector<WatingPlayerData> rematchPd;

	for (auto& pd : pdv) {
		shared_ptr<PlayerSession> playerSessionRef = pd.playerSessionWRef.lock();

		if (playerSessionRef == nullptr) {
			ready = false;
			continue;
		}
		//�� Session�� KeepAlive��Ŷ�� BroadCast.
		S2C_Protocol::S_MatchmakeKeepAlive pkt = S2CPacketMaker::MakeSMatchmakeKeepAlive(2);
		shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
		playerSessionRef->Send(sendBuffer);
		rematchPd.push_back(pd);
	}

	if (ready) {
		//1�� ��, (Ping�� 1�ʰ� �Ѵ°���, �̻��ϴ�.) ��� ��Ŷ���κ��� ������ �޾Ҵٸ� ����
		PostEventAfter(1000, &PingPongGameRoom::Init2, move(pdv));
	}
	else {
		GGameManagers[2]->Push(rematchPd);
	}
}

void PingPongGameRoom::Init2(vector<WatingPlayerData> pdv) {

}

void PingPongGameRoom::ReturnToPool() {
	objectPool<PingPongGameRoom>::dealloc(this);
}

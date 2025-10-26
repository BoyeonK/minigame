#include "pch.h"
#include "GameRoom.h"
#include "S2CPacketHandler.h"
#include "S2CPacketMaker.h"

void GameRoom::SetRoomId(int32_t roomId) {
	_roomId = roomId;
}

int32_t GameRoom::GetRoomId() const {
	return _roomId;
}

GameRoom::GameState GameRoom::GetState() const {
	return _state;
}

int32_t GameRoom::GenerateUniqueGameObjectId() {
	return _nxtObjectId++;
}

void GameRoom::RegisterGameObject(const shared_ptr<UnityGameObject>& obj) {
	_vecGameObjects.push_back(obj);
	_hmGameObjects.insert({ obj->GetObjectId(), obj});
}

void GameRoom::BroadCast(shared_ptr<SendBuffer> sendBuffer) {
	if (sendBuffer == nullptr)
		return;
	for (auto& playerWRef : _playerWRefs) {
		shared_ptr<PlayerSession> playerRef = playerWRef.lock();
		if (playerRef == nullptr)
			continue;
		playerRef->Send(sendBuffer);
	}
}

void GameRoom::BroadCastSpawn(const shared_ptr<UnityGameObject>& objRef) {
	if (objRef == nullptr)
		return;
	S2C_Protocol::S_SpawnGameObject pkt = S2CPacketMaker::MakeSSpawnGameObject(objRef);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

void GameRoom::BroadCastDespawn(const shared_ptr<UnityGameObject>& objRef) {
	if (objRef == nullptr)
		return;
	S2C_Protocol::S_SpawnGameObject pkt = S2CPacketMaker::MakeSSpawnGameObject(objRef);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(pkt);
	BroadCast(sendBuffer);
}

double GameRoom::Winrate(int32_t elo1, int32_t elo2) {
	double denominator = (pow(10, (elo2 - elo1) / 400) + 1);
	return 1.0 / denominator;
}

int32_t GameRoom::CalculateEloW(int32_t winnerElo, int32_t opponentElo) {
	double deltaElo = 20 * (1 - Winrate(winnerElo, opponentElo));
	return static_cast<int32_t>(winnerElo + deltaElo);
}

int32_t GameRoom::CalculateEloL(int32_t loserElo, int32_t opponentElo) {
	double deltaElo = -20 * Winrate(loserElo, opponentElo);
	return static_cast<int32_t>(loserElo + deltaElo);
}

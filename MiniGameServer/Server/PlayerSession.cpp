#include "pch.h"
#include "PlayerSession.h"
#include "ServerGlobal.h"
#include "S2CPacketMaker.h"
#include "S2CPacketHandler.h"

void PlayerSession::OnConnected() {
	cout << "Player Session Onconnected!" << endl;

	EVP_PKEY* rawKey = GCryptoManager->PopKey();
	if (!rawKey) {
		cout << "Failed to Pop RSAKey!" << endl;
		Disconnect();
		return;
	}

	vector<unsigned char> publicKey = GCryptoManager->ExtractPublicKey(rawKey);
	_RSAKey = EVP_PKEY_dup(rawKey);
	GCryptoManager->ReturnKey(rawKey);
	if (!_RSAKey) {
		cout << "Failed to duplicate RSAKey!" << endl;
		Disconnect();
		return;
	}

	S2C_Protocol::S_Welcome sendPkt = S2CPacketMaker::MakeSWelcome(publicKey, _gameVersion);
	shared_ptr<SendBuffer> sendBuffer = S2CPacketHandler::MakeSendBufferRef(sendPkt);
	Send(sendBuffer);
}

void PlayerSession::OnDisconnected() {
	cout << "Player Session OnDisconnected!" << endl;
}

void PlayerSession::OnRecvPacket(unsigned char* buffer, int32_t len) {
	S2CPacketHandler::HandlePacket(static_pointer_cast<PBSession>(shared_from_this()), buffer, len);
}

EVP_PKEY* PlayerSession::GetRSAKey() {
	return _RSAKey;
}

void PlayerSession::SetAESKey(vector<unsigned char>&& AESKey) {
	_AESKey = move(AESKey);

	//디버그용. 추후 삭제 예정.
	cout << "SetAESKey (Move)" << endl;
}

void PlayerSession::SetAESKey(vector<unsigned char>& AESKey) {
	_AESKey = AESKey;

	//디버그용. 추후 삭제 예정.
	cout << "SetAESKey (Copy)" << endl;
}

vector<unsigned char> PlayerSession::GetAESKey() {
	return _AESKey;
}

void PlayerSession::SetSecureLevel(int32_t lv) {
	_secureLevel = lv;
}

void PlayerSession::SetDbid(int32_t dbid) {
	_dbid = dbid;
}

void PlayerSession::SetMatchingState(const GameType& value) {
	_matchingState = value;
}

bool PlayerSession::TryChangeMatchingState(GameType& expected, GameType desired) {
	return _matchingState.compare_exchange_strong(expected, desired);
}

void PlayerSession::SetRoomIdx(const int32_t roomIdx) {
	_roomIdx = roomIdx;
}

int32_t PlayerSession::GetRoomIdx() {
	return _roomIdx;
}

void PlayerSession::SetJoinedRoom(shared_ptr<GameRoom> roomRef) {
	_joinedRoomWRef = roomRef;
}

shared_ptr<GameRoom> PlayerSession::GetJoinedRoom() {
	return _joinedRoomWRef.lock();
}

void PlayerSession::SetElo(int32_t idx, int32_t elo) {
	_elos[idx] = elo;
}

int32_t PlayerSession::GetElo(const int32_t& idx) const {
	return _elos[idx];
}

void PlayerSession::SetPersonalRecord(int32_t idx, int32_t score) {
	cout << "퍼스널레코드" << idx << " " << score << endl;
	_personalRecords[idx] = score;
}

int32_t PlayerSession::GetPersonalRecord(const int32_t& idx) const {
	return _personalRecords[idx];
}

int64_t PlayerSession::GetLastKeepAliveTick() const {
	return _lastKeepAliveTick;
}

void PlayerSession::SetLastKeepAliveTick(const int64_t& tick) {
	_lastKeepAliveTick.store(tick);
}

string PlayerSession::GetPlayerId() const {
	return _playerId;
}

void PlayerSession::SetPlayerId(const string& playerId) {
	_playerId = playerId;
}



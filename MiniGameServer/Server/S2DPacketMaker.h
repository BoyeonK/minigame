#pragma once
class S2DPacketMaker {
public:
	static S2D_Protocol::S2D_Login Make_S2D_Login(string id, string password) {
		S2D_Protocol::S2D_Login pkt;
		pkt.set_id(id);
		pkt.set_password(password);
		return pkt;
	}

	static S2D_Protocol::S2D_CreateAccount Make_S2D_CreateAccount(string id, string password) {
		S2D_Protocol::S2D_CreateAccount pkt;
		pkt.set_id(id);
		pkt.set_password(password);
		return pkt;
	}

	static S2D_Protocol::S2D_RequestPlayerInfomation Make_S2D_RequestPlayerInfomation(int32_t dbid) {
		S2D_Protocol::S2D_RequestPlayerInfomation pkt;
		pkt.set_dbid(dbid);
		return pkt;
	}

	static S2D_Protocol::S2D_TryUpdateElo Make_S2D_TryUpdateElo(int32_t dbid, int32_t gameId, int32_t elo) {
		S2D_Protocol::S2D_TryUpdateElo pkt;
		pkt.set_dbid(dbid);
		pkt.set_gameid(gameId);
		pkt.set_elo(elo);
		return pkt;
	}
	
	static S2D_Protocol::S2D_TryUpdatePersonalRecord Make_S2D_TryUpdatePersonalRecord(int32_t dbid, int32_t gameId, int32_t score) {
		S2D_Protocol::S2D_TryUpdatePersonalRecord pkt;
		pkt.set_dbid(dbid);
		pkt.set_gameid(gameId);
		pkt.set_score(score);
		return pkt;
	}

	static S2D_Protocol::S2D_RequestPublicRecord Make_S2D_RequestPublicRecord(int32_t gameId) {
		S2D_Protocol::S2D_RequestPublicRecord pkt;
		pkt.set_gameid(gameId);
		return pkt;
	}

	static S2D_Protocol::S2D_TryUpdatePublicRecord Make_S2D_TryUpdatePublicRecord(int32_t gameId, int32_t dbid, int32_t score) {
		S2D_Protocol::S2D_TryUpdatePublicRecord pkt;
		pkt.set_gameid(gameId);
		pkt.set_recordersdbid(dbid);
		pkt.set_recordersscore(score);
		return pkt;
	}
};


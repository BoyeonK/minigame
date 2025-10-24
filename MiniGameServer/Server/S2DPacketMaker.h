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

	static S2D_Protocol::S2D_RequestPlayerInfomation Make_S2D_RequestPlayerInfomation(int dbid) {
		S2D_Protocol::S2D_RequestPlayerInfomation pkt;
		pkt.set_dbid(dbid);
		return pkt;
	}
};


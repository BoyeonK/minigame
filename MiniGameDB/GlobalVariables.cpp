#include "GlobalVariables.h"

DBManager* GDBManager = nullptr;

class DBGlobal {
public:
	DBGlobal() {
		GDBManager = new DBManager();
	}

	~DBGlobal() {
		delete GDBManager;
	}

} GDBGlobal;

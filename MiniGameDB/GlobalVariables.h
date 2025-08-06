#include <queue>

extern class DBManager* GDBManager;
extern class ThreadManager* GThreadManager;

class DBManager {
public:
    DBManager();
    ~DBManager();

    static const int pid_size = 16;
    static const int salt_size = 16;
    static const int hash_size = 32;
    static const int pbkdf2_iter = 10000;
    
    bool CheckReturn(SQLRETURN& ret);
    bool CheckReturn(SQLRETURN& ret, SQLHANDLE hHandle, SQLSMALLINT HandleType);

    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);
    wstring v2wsRef(const vector<unsigned char>& in_binary);

    //이대로 사용하면 코드인젝션에 굉장히 취약함!
    wstring CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs);


    SQLHENV getHEnv() { return _hEnv; }
    SQLHDBC connectNewHDbc();
    SQLHDBC popHDbc();
    void returnHDbc(SQLHDBC hDbc);

private:
    mutex _mtx;
	SQLHENV _hEnv;
    queue<SQLHDBC> _hDbcQ;

    //생성자에서 CRUD 테스트를 위해 사용할 함수들
    void InitialC();
    void InitialR();
    void InitialU();
    void InitialD();
};

class ThreadManager {
public:
    ThreadManager();
    ~ThreadManager();

    static void InitTLS();
    static void DestroyTLS() { };
    //이후에 JobQueue가 필요한 시점이 오면 그때 추가하는 걸로.
    //static void DoGlobalQueueWork();
    //static void DoTimerQueueDistribution();

    void Launch(function<void()> callback);
    void Join();

private:
    mutex	_mutex;
    vector<thread> _threads;
};

struct SQLHandleDeleter {
    SQLSMALLINT handleType;
    SQLHANDLE handle;
    void operator()(SQLHANDLE* pHandle) {
        if (pHandle && *pHandle != SQL_NULL_HANDLE) {
            SQLFreeHandle(handleType, *pHandle);
            delete pHandle;
        }
    }
};

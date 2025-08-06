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

    //�̴�� ����ϸ� �ڵ������ǿ� ������ �����!
    wstring CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs);


    SQLHENV getHEnv() { return _hEnv; }
    SQLHDBC connectNewHDbc();
    SQLHDBC popHDbc();
    void returnHDbc(SQLHDBC hDbc);

private:
    mutex _mtx;
	SQLHENV _hEnv;
    queue<SQLHDBC> _hDbcQ;

    //�����ڿ��� CRUD �׽�Ʈ�� ���� ����� �Լ���
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
    //���Ŀ� JobQueue�� �ʿ��� ������ ���� �׶� �߰��ϴ� �ɷ�.
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

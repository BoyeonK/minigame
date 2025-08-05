extern class DBManager* GDBManager;
extern class ThreadManager* GThreadManager;

class DBManager {
public:
    DBManager();
    ~DBManager();

    static const int pid_size = 16;
    static const int salt_size = 16;
    static const int hash_size = 16;
    static const int pbkdf2_iter = 10000;
    
    bool CheckReturn(SQLRETURN& ret);
    bool CheckReturn(SQLRETURN& ret, SQLHANDLE hHandle, SQLSMALLINT HandleType);

    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);

    //�̴�� ����ϸ� �ڵ������ǿ� ������ �����!
    wstring CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs);

    /*
    wchar_t* ReadQueryA2W();
    wchar_t* UpdateQueryA2W();
    wchar_t* DeleteQueryA2W();
    */

    SQLHENV getHEnv() { return _hEnv; }
    SQLHDBC getHDbc() { return _hDbc; }

private:
	SQLHENV _hEnv;
	SQLHDBC _hDbc;

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

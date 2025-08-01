extern class DBManager* GDBManager;
extern class ThreadManager* GThreadManager;

class DBManager {
public:
    DBManager();
    ~DBManager();
    
    bool CheckReturn(SQLRETURN& ret);
    bool CheckReturn(SQLRETURN& ret, SQLHANDLE hHandle, SQLSMALLINT HandleType);

    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);

    //이대로 사용하면 코드인젝션에 굉장히 취약함!
    wstring CreateQuery(const wstring& tableName, initializer_list<wstring> wstrs);

    /*
    wchar_t* ReadQueryA2W();
    wchar_t* UpdateQueryA2W();
    wchar_t* DeleteQueryA2W();
    */

private:
	SQLHENV _hEnv;
	SQLHDBC _hDbc;

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

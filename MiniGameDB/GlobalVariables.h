#pragma once
#include <queue>

extern class DBManager* GDBManager;
extern class ThreadManager* GThreadManager;
extern thread_local SQLHDBC LhDbc;

class DBManager {
public:
    DBManager();
    ~DBManager();

    static const int pid_size = 16;
    static const int salt_size = 16;
    static const int hash_size = 32;
    static const int pbkdf2_iter = 10000;

    //초기화 시, 환경변수 세팅 함수.
    void SetEnv();
    
    //에러 핸들링 함수.
    bool CheckReturn(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle);

    //문자열 인코딩 함수들
    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);
    wstring v2wsRef(const vector<unsigned char>& in_binary);
    vector<unsigned char> s2vRef(const string& hex_str);
    vector<unsigned char> ws2vRef(const wstring& ws);
    string ws2sRef(const wstring& in_u16ws);
    //string ws2aRef

    //환경변수 pooling함수.
    SQLHENV GetHEnv() { return _hEnv; }
    SQLHDBC ConnectNewHDbc();
    SQLHDBC PopHDbc();
    void ReturnHDbc(SQLHDBC hDbc);

    //자주 쓰는 친구들 함수로 묶음.
    void PrepareQ(SQLHSTMT& hStmt, const wstring& query);
    void BindPInt(SQLHSTMT& hStmt, const int& param, const int val);
    void BindPWchar(SQLHSTMT& hStmt, const int& param, const wstring& ws);

private:
    mutex _mtx;
	SQLHENV _hEnv;
    queue<SQLHDBC> _hDbcQ;

    //생성자에서 CRUD 테스트를 위해 사용할 함수들
    void InitialC();
    void InitialR();
    void InitialU();
    void InitialD();
    void AkagiRedSunsNo2();
    void ScandinavianFlick();
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

class Cleaner {
public:
    //암시적 형 변환을 때려막는다. 가끔 ㄹㅇ 병신크리가 뜰수있다.
    explicit Cleaner(std::function<void()> cleanup_func)
        : m_cleanup_function(cleanup_func) {
    }

    ~Cleaner() {
        if (!m_dismissed) {
            m_cleanup_function();
        }
    }

    void dismiss() {
        m_dismissed = true;
    }

    // 복사 생성자 및 대입 연산자 금지
    Cleaner(const Cleaner&) = delete;
    Cleaner& operator=(const Cleaner&) = delete;

private:
    std::function<void()> m_cleanup_function;
    bool m_dismissed = false;
};

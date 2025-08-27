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

    //�ʱ�ȭ ��, ȯ�溯�� ���� �Լ�.
    void SetEnv();
    
    //���� �ڵ鸵 �Լ�.
    bool CheckReturn(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle);

    //���ڿ� ���ڵ� �Լ���
    wstring a2wsRef(const string& in_cp949);
    wstring s2wsRef(const string& in_u8s);
    wstring v2wsRef(const vector<unsigned char>& in_binary);
    vector<unsigned char> s2vRef(const string& hex_str);
    vector<unsigned char> ws2vRef(const wstring& ws);
    string ws2sRef(const wstring& in_u16ws);
    //string ws2aRef

    //ȯ�溯�� pooling�Լ�.
    SQLHENV GetHEnv() { return _hEnv; }
    SQLHDBC ConnectNewHDbc();
    SQLHDBC PopHDbc();
    void ReturnHDbc(SQLHDBC hDbc);

    //���� ���� ģ���� �Լ��� ����.
    void PrepareQ(SQLHSTMT& hStmt, const wstring& query);
    void BindPInt(SQLHSTMT& hStmt, const int& param, const int val);
    void BindPWchar(SQLHSTMT& hStmt, const int& param, const wstring& ws);

private:
    mutex _mtx;
	SQLHENV _hEnv;
    queue<SQLHDBC> _hDbcQ;

    //�����ڿ��� CRUD �׽�Ʈ�� ���� ����� �Լ���
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
    //���Ŀ� JobQueue�� �ʿ��� ������ ���� �׶� �߰��ϴ� �ɷ�.
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
    //�Ͻ��� �� ��ȯ�� �������´�. ���� ���� ����ũ���� ����ִ�.
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

    // ���� ������ �� ���� ������ ����
    Cleaner(const Cleaner&) = delete;
    Cleaner& operator=(const Cleaner&) = delete;

private:
    std::function<void()> m_cleanup_function;
    bool m_dismissed = false;
};

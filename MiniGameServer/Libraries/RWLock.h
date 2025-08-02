#pragma once
#include "pch.h"
#include <atomic>

//std::shared_mutex와 사실상 동일한 기능을 한다.
class RWLock {
	enum : uint32_t {
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000,
	};

public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

private:
	atomic<uint32_t> _lockFlag = EMPTY_FLAG;
	uint32_t _writeCount = 0;
};

class ReadLockGuard {
public:
	ReadLockGuard(RWLock& lock) : _lock(lock) { _lock.ReadLock(); }
	~ReadLockGuard() { _lock.ReadUnlock(); }
private:
	RWLock& _lock;
};

class WriteLockGuard {
public:
	WriteLockGuard(RWLock& lock) : _lock(lock) { _lock.WriteLock(); }
	~WriteLockGuard() { _lock.WriteUnlock(); }
private:
	RWLock& _lock;
};


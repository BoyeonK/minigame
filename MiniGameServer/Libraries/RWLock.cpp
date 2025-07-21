#include "pch.h"
#include "RWLock.h"
#include "CoreGlobal.h"
#include <thread>

//#include <sysinfoapi.h> Windows에 종속적
#include <time.h>

void RWLock::WriteLock() {

	//동일한 Thread가 WriteLockGuard를 이미 소유하고 있는 경우
	const uint32_t TaskThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (MyThreadID == TaskThreadId) {
		_writeCount++;
		return;
	}
	clock_t S_tick = clock();
	//uint64_t S_tick = GetTickCount64();

	//이외의 경우, _lockFlag가 0x0000'0000일 때 까지 race condition으로 들어간다.
	const uint32_t desired = (MyThreadID << 16 & WRITE_THREAD_MASK);
	while (true) {
		for (uint32_t trial = 0; trial < MAX_SPIN_COUNT; trial++) {
			uint32_t expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired)) {
				_writeCount++;
				return;
			}
		}
		//if (GetTickCount64() - S_tick > ACQUIRE_TIMEOUT_TICK) {
		if (clock() - S_tick > ACQUIRE_TIMEOUT_TICK) {
			CRASH("spinLock 시간 초과");
			//TODO :: 최대 소요시간 초과시 예외처리
		}
		//지정된 횟수 이상의 spin이후 재스케줄링 대상으로 편입
		this_thread::yield();
	}
}

void RWLock::WriteUnlock() {
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0) {
		//Read 작업이 완료되지 않은 경우에 WriteLock이 해제되는것은 자연스럽지 않다.
		//Write작업 도중의 Read우려가 있으므로 Crash를 일으킨다.
		CRASH("WriteLock 해제 오류. ReadLock이 먼저 해제 되지 않음.")
	}

	const int32_t lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void RWLock::ReadLock() {
	//동일한 Thread가 WriteLockGuard를 이미 소유하고 있는 경우
	const uint32_t TaskThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (MyThreadID == TaskThreadId) {
		_lockFlag.fetch_add(1);
		return;
	}

	clock_t S_tick = clock();
	//uint64_t S_tick = GetTickCount64();

	//이외의 경우, _lockFlag가 0x0000'XXXX일 때 까지
	//race condition으로 들어간다. Read끼리는 경합하지 않는다만,
	//CAS연산시 다른 Thread가 WriteLock을 획득한 경우를 조심한다.
	while (true) {
		for (uint32_t trial = 0; trial < MAX_SPIN_COUNT; trial++) {
			uint32_t expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(expected, expected + 1)) {
				return;
			}
		}
		//if (GetTickCount64() - S_tick > ACQUIRE_TIMEOUT_TICK) {
		if (clock() - S_tick > ACQUIRE_TIMEOUT_TICK) {
			CRASH("spinLock 시간 초과");
			//TODO :: 최대 소요시간 초과시 예외처리
		}
		//지정된 횟수 이상의 spin이후 재스케줄링 대상으로 편입
		this_thread::yield();
	}
}

void RWLock::ReadUnlock() {
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("ReadCount가 이미 0이어서 해제할 수 없음");
}

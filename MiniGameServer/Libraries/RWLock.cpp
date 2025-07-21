#include "pch.h"
#include "RWLock.h"
#include "CoreGlobal.h"
#include <thread>

//#include <sysinfoapi.h> Windows�� ������
#include <time.h>

void RWLock::WriteLock() {

	//������ Thread�� WriteLockGuard�� �̹� �����ϰ� �ִ� ���
	const uint32_t TaskThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (MyThreadID == TaskThreadId) {
		_writeCount++;
		return;
	}
	clock_t S_tick = clock();
	//uint64_t S_tick = GetTickCount64();

	//�̿��� ���, _lockFlag�� 0x0000'0000�� �� ���� race condition���� ����.
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
			CRASH("spinLock �ð� �ʰ�");
			//TODO :: �ִ� �ҿ�ð� �ʰ��� ����ó��
		}
		//������ Ƚ�� �̻��� spin���� �罺���ٸ� ������� ����
		this_thread::yield();
	}
}

void RWLock::WriteUnlock() {
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0) {
		//Read �۾��� �Ϸ���� ���� ��쿡 WriteLock�� �����Ǵ°��� �ڿ������� �ʴ�.
		//Write�۾� ������ Read����� �����Ƿ� Crash�� ����Ų��.
		CRASH("WriteLock ���� ����. ReadLock�� ���� ���� ���� ����.")
	}

	const int32_t lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void RWLock::ReadLock() {
	//������ Thread�� WriteLockGuard�� �̹� �����ϰ� �ִ� ���
	const uint32_t TaskThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (MyThreadID == TaskThreadId) {
		_lockFlag.fetch_add(1);
		return;
	}

	clock_t S_tick = clock();
	//uint64_t S_tick = GetTickCount64();

	//�̿��� ���, _lockFlag�� 0x0000'XXXX�� �� ����
	//race condition���� ����. Read������ �������� �ʴ´ٸ�,
	//CAS����� �ٸ� Thread�� WriteLock�� ȹ���� ��츦 �����Ѵ�.
	while (true) {
		for (uint32_t trial = 0; trial < MAX_SPIN_COUNT; trial++) {
			uint32_t expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(expected, expected + 1)) {
				return;
			}
		}
		//if (GetTickCount64() - S_tick > ACQUIRE_TIMEOUT_TICK) {
		if (clock() - S_tick > ACQUIRE_TIMEOUT_TICK) {
			CRASH("spinLock �ð� �ʰ�");
			//TODO :: �ִ� �ҿ�ð� �ʰ��� ����ó��
		}
		//������ Ƚ�� �̻��� spin���� �罺���ٸ� ������� ����
		this_thread::yield();
	}
}

void RWLock::ReadUnlock() {
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("ReadCount�� �̹� 0�̾ ������ �� ����");
}

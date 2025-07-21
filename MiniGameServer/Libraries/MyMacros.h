#pragma once
#include <cstdint>

#define OUT

#define USE_RWLOCKS(count)		RWLock _locks[count];
#define USE_RWLOCK				USE_RWLOCKS(1)
#define READ_RWLOCKS(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx]);
#define READ_RWLOCK				READ_RWLOCKS(0)
#define WRITE_RWLOCKS(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx]);
#define WRITE_RWLOCK			WRITE_RWLOCKS(0)

#define CRASH(cause) {						\
	uint32_t* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr) {		\
	if (!(expr)) {					\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}
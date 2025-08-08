#pragma once

using namespace std;

#ifdef _DEBUG
class counter {
public:
	atomic<uint32_t> _uses = 0;
	atomic<uint32_t> _reserves = 0;
};
#endif

template<typename _Ty>
class poolHeader {
public:
	poolHeader() {
		::InitializeSListHead(&_header);
	}

	~poolHeader() {
		PSLIST_ENTRY pSE;
		while (pSE = ::InterlockedPopEntrySList(&_header)) {
			_Ty* ptr = reinterpret_cast<_Ty*>(++pSE);
			ptr->~_Ty();
			_aligned_free(pSE);
		}
	}

	//aligned_malloc은 '동적으로' 16배수로 맞춘다.
	//이후에 다시 objectPool을 설계할 일이 있다면, alignas 키워드를 사용하는 쪽이 바람직함. 지금은 굳이 동작하는 코드를 건드리지 말자.
	PSLIST_ENTRY popEntry(uint32_t _typeSize) {
		PSLIST_ENTRY pEntry = ::InterlockedPopEntrySList(&_header);
		if (pEntry == nullptr)
			pEntry = reinterpret_cast<PSLIST_ENTRY>(_aligned_malloc(sizeof(SLIST_ENTRY) + _typeSize, 16));
		return pEntry;
	}

	void pushEntry(PSLIST_ENTRY pSE) {
		::InterlockedPushEntrySList(&_header, pSE);
	}

private:
	alignas(16) SLIST_HEADER _header;
};

template<typename _Ty>
class objectPool {
public:
	template<typename... Args>
	static _Ty* alloc(Args&&... args) {
		PSLIST_ENTRY pSE = _poolHeader.popEntry(_typeSize);
		_Ty* ptr = reinterpret_cast<_Ty*>(++pSE);
		new(ptr)_Ty(forward<Args>(args)...);
#ifdef _DEBUG
		_counter._uses.fetch_add(1);
#endif
		return ptr;
	}

	static void dealloc(_Ty* ptr) {
#ifdef _DEBUG
		_counter._uses.fetch_sub(1);
		_counter._reserves.fetch_add(1);
#endif
		PSLIST_ENTRY pSE = reinterpret_cast<PSLIST_ENTRY>(ptr) - 1;
		_poolHeader.pushEntry(pSE);
		ptr->~_Ty();
	}

public:
#ifdef _DEBUG
	static counter _counter;
#endif

private:
	static poolHeader<_Ty> _poolHeader;
	static uint32_t _typeSize;
};

template<typename _Ty>
uint32_t objectPool<_Ty>::_typeSize = sizeof(_Ty);

template<typename _Ty>
poolHeader<_Ty> objectPool<_Ty>::_poolHeader{};

#ifdef _DEBUG
template<typename _Ty>
counter objectPool<_Ty>::_counter{};
#endif
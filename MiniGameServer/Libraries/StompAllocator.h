#pragma once

class StompAllocator {
	enum {
		PAGE_SIZE = 0x1000,
	};

public:
	static void*	Alloc(uint32_t size);
	static void		Release(void* ptr);
};

template<class _Ty>
class STLAllocator {
public:
	//1. 요소의 타입
	using value_type = _Ty;

	//2. 생성자
	STLAllocator() {}

	template<class Other>
	STLAllocator(const STLAllocator<Other>&) {}

	//3. Data배열을 할당할 방법
	_Ty* allocate(size_t count) {
		const uint32_t size = static_cast<uint32_t>(count * sizeof(_Ty));
		return static_cast<_Ty*>(StompAllocator::Alloc(size));
	}

	//4. 해제할 방법
	void deallocate(_Ty* ptr, size_t count) {
		StompAllocator::Release(ptr);
	}
};


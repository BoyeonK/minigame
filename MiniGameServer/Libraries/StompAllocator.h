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
	//1. ����� Ÿ��
	using value_type = _Ty;

	//2. ������
	STLAllocator() {}

	template<class Other>
	STLAllocator(const STLAllocator<Other>&) {}

	//3. Data�迭�� �Ҵ��� ���
	_Ty* allocate(size_t count) {
		const uint32_t size = static_cast<uint32_t>(count * sizeof(_Ty));
		return static_cast<_Ty*>(StompAllocator::Alloc(size));
	}

	//4. ������ ���
	void deallocate(_Ty* ptr, size_t count) {
		StompAllocator::Release(ptr);
	}
};


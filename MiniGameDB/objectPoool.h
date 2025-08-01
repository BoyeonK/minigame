#pragma once

#include <windows.h>
#include <forward_list> // 메모리 블록 관리를 위해
#include <memory>
#include <utility>

// 메모리 블록 헤더 + 실제 객체 데이터 구조체
template<typename _Ty>
struct PoolNode {
    SLIST_ENTRY entry;
    alignas(std::max(alignof(_Ty), alignof(SLIST_ENTRY))) char data[sizeof(_Ty)];
};

template<typename _Ty>
class objectPool {
public:
    // 풀의 초기 크기를 지정하는 생성자
    objectPool(size_t initialSize = 0) {
        ::InitializeSListHead(&_header);
        if (initialSize > 0) {
            _reserve(initialSize);
        }
    }

    // 소멸자에서 풀에 남아있는 모든 메모리 블록 해제
    ~objectPool() {
        PSLIST_ENTRY pSE;
        while ((pSE = ::InterlockedPopEntrySList(&_header)) != nullptr) {
            PoolNode<_Ty>* pNode = reinterpret_cast<PoolNode<_Ty>*>(pSE);
            delete pNode; // new로 할당한 메모리를 delete로 해제
        }
    }

    template<typename... Args>
    _Ty* alloc(Args&&... args) {
        PSLIST_ENTRY pSE = ::InterlockedPopEntrySList(&_header);
        if (pSE == nullptr) {
            // 풀이 비어있을 때 동적으로 하나 더 할당
            PoolNode<_Ty>* pNode = new PoolNode<_Ty>();
            pSE = &(pNode->entry);
        }

        _Ty* ptr = reinterpret_cast<_Ty*>(&reinterpret_cast<PoolNode<_Ty>*>(pSE)->data);
        new(ptr) _Ty(std::forward<Args>(args)...); // placement new
        return ptr;
    }

    void dealloc(_Ty* ptr) {
        // 소멸자를 호출하여 객체 상태 정리
        ptr->~_Ty();

        // 포인터 연산의 위험성을 줄이기 위해 PoolNode*로 변환
        PoolNode<_Ty>* pNode = reinterpret_cast<PoolNode<_Ty>*>(reinterpret_cast<char*>(ptr) - offsetof(PoolNode<_Ty>, data));

        // 다시 SList에 푸시
        ::InterlockedPushEntrySList(&_header, &pNode->entry);
    }

private:
    void _reserve(size_t count) {
        for (size_t i = 0; i < count; ++i) {
            PoolNode<_Ty>* pNode = new PoolNode<_Ty>();
            ::InterlockedPushEntrySList(&_header, &pNode->entry);
        }
    }

private:
    alignas(16) SLIST_HEADER _header;
};

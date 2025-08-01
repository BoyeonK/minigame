#pragma once

#include <windows.h>
#include <forward_list> // �޸� ��� ������ ����
#include <memory>
#include <utility>

// �޸� ��� ��� + ���� ��ü ������ ����ü
template<typename _Ty>
struct PoolNode {
    SLIST_ENTRY entry;
    alignas(std::max(alignof(_Ty), alignof(SLIST_ENTRY))) char data[sizeof(_Ty)];
};

template<typename _Ty>
class objectPool {
public:
    // Ǯ�� �ʱ� ũ�⸦ �����ϴ� ������
    objectPool(size_t initialSize = 0) {
        ::InitializeSListHead(&_header);
        if (initialSize > 0) {
            _reserve(initialSize);
        }
    }

    // �Ҹ��ڿ��� Ǯ�� �����ִ� ��� �޸� ��� ����
    ~objectPool() {
        PSLIST_ENTRY pSE;
        while ((pSE = ::InterlockedPopEntrySList(&_header)) != nullptr) {
            PoolNode<_Ty>* pNode = reinterpret_cast<PoolNode<_Ty>*>(pSE);
            delete pNode; // new�� �Ҵ��� �޸𸮸� delete�� ����
        }
    }

    template<typename... Args>
    _Ty* alloc(Args&&... args) {
        PSLIST_ENTRY pSE = ::InterlockedPopEntrySList(&_header);
        if (pSE == nullptr) {
            // Ǯ�� ������� �� �������� �ϳ� �� �Ҵ�
            PoolNode<_Ty>* pNode = new PoolNode<_Ty>();
            pSE = &(pNode->entry);
        }

        _Ty* ptr = reinterpret_cast<_Ty*>(&reinterpret_cast<PoolNode<_Ty>*>(pSE)->data);
        new(ptr) _Ty(std::forward<Args>(args)...); // placement new
        return ptr;
    }

    void dealloc(_Ty* ptr) {
        // �Ҹ��ڸ� ȣ���Ͽ� ��ü ���� ����
        ptr->~_Ty();

        // ������ ������ ���輺�� ���̱� ���� PoolNode*�� ��ȯ
        PoolNode<_Ty>* pNode = reinterpret_cast<PoolNode<_Ty>*>(reinterpret_cast<char*>(ptr) - offsetof(PoolNode<_Ty>, data));

        // �ٽ� SList�� Ǫ��
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

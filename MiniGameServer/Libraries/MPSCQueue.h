#pragma once

template<typename T>
class LockQueue {
public:
	void Push(T item) {
		WRITE_RWLOCK;
		_items.push(item);
	}

	T Pop() {
		WRITE_RWLOCK;
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}

	void PopAll(vector<T>& items) {
		WRITE_RWLOCK;
		while (T item = Pop()) {
			items.push_back(item);
		}
	}

	void Clear() {
		WRITE_RWLOCK;
		_items = queue<T>();
	}

private:
	USE_RWLOCK;
	queue<T> _items;
};

template<typename T>
class MPSCQueue {
private:
    struct Node {
        Node() = default; 
        Node(T&& data) : _data(move(data)) { } 

        T _data;
        std::atomic<Node*> next = nullptr;
    };

public:
    MPSCQueue() {
        Node* dummy = new Node();
        _head.store(dummy);
        _tail.store(dummy);
    }

    ~MPSCQueue() {
        T out_item;
        while (TryPop(out_item)) { }
        delete _head.load();
    }

    void Push(T item) {
        Node* newNode = new Node(move(item));
        Node* prevTail = _tail.exchange(newNode, std::memory_order_acq_rel);
        prevTail->next.store(newNode, std::memory_order_release);
    }

    bool TryPop(T& outItem) {
        Node* head = _head.load(std::memory_order_relaxed);
        Node* next = head->next.load(std::memory_order_acquire);

        if (next == nullptr) {
            return false;
        }

        outItem = move(next->_data);
        _head.store(next, std::memory_order_release);
        delete head;
        return true;
    }

private:
    atomic<Node*> _head;
    atomic<Node*> _tail;
};
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
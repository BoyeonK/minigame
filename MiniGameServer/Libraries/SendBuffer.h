#pragma once
#include <array>

class SendBuffer;

class SendBufferChunk : public enable_shared_from_this<SendBufferChunk> {
public:
	enum {
		SEND_BUFFER_CHUNK_SIZE = 6000,
	};
	SendBufferChunk() {	Init(); }

	void Init();
	shared_ptr<SendBuffer> Open(uint32_t allocSize);
	void Close(uint32_t allocSize);

	bool IsOpen() { return _isOpen; };
	unsigned char* Index() { return &_buffer[_usedSize]; }

	//std::array<_ty>.size()가 int64를 반환하기 때문에
	//형변환을 해주지 않는다면 데이터 손실을 야기할 수 있다.
	uint32_t FreeSize() { return static_cast<uint32_t>(_buffer.size() - _usedSize); }

private:
	array<unsigned char, SEND_BUFFER_CHUNK_SIZE> _buffer = {};
	//SendBufferChunk는 TLS로 사용할 것이기 때문에 thread-safe
	//atomic으로 만들어 줄 필요가 없다.
	bool _isOpen;
	uint32_t _usedSize;
};

class SendBufferManager {
	enum {
		SEND_BUFFER_CHUNK_SIZE = 6000,
	};

public:
	shared_ptr<SendBuffer> Open(uint32_t allockSize);
};

class SendBuffer {
public:
	SendBuffer() { }
	~SendBuffer() { }
	//나는 SendBuffer를 pool을 통해 관리중이다. 재사용시 초기화 함수
	void Init(shared_ptr<SendBufferChunk> chunkRef, unsigned char* index, uint32_t allocSize);

	unsigned char* Buffer() { return _index; }
	uint32_t AllocSize() { return _allocSize; }
	uint32_t WriteSize() { return _writeSize; }
	void Close(uint32_t writeSize);

private:
	shared_ptr<SendBufferChunk> _chunkRef;
	unsigned char* _index = nullptr;

	//처음 SendBuffer를 생성하면서, 쓰겠다고 선언한 값 (널널하게 부를 수 있다.)
	uint32_t _allocSize = 0;

	//실제 덮어쓴 값
	uint32_t _writeSize = 0;
};
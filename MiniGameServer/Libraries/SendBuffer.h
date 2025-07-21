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

	//std::array<_ty>.size()�� int64�� ��ȯ�ϱ� ������
	//����ȯ�� ������ �ʴ´ٸ� ������ �ս��� �߱��� �� �ִ�.
	uint32_t FreeSize() { return static_cast<uint32_t>(_buffer.size() - _usedSize); }

private:
	array<unsigned char, SEND_BUFFER_CHUNK_SIZE> _buffer = {};
	//SendBufferChunk�� TLS�� ����� ���̱� ������ thread-safe
	//atomic���� ����� �� �ʿ䰡 ����.
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
	//���� SendBuffer�� pool�� ���� �������̴�. ����� �ʱ�ȭ �Լ�
	void Init(shared_ptr<SendBufferChunk> chunkRef, unsigned char* index, uint32_t allocSize);

	unsigned char* Buffer() { return _index; }
	uint32_t AllocSize() { return _allocSize; }
	uint32_t WriteSize() { return _writeSize; }
	void Close(uint32_t writeSize);

private:
	shared_ptr<SendBufferChunk> _chunkRef;
	unsigned char* _index = nullptr;

	//ó�� SendBuffer�� �����ϸ鼭, ���ڴٰ� ������ �� (�γ��ϰ� �θ� �� �ִ�.)
	uint32_t _allocSize = 0;

	//���� ��� ��
	uint32_t _writeSize = 0;
};
#include "pch.h"
#include "SendBuffer.h"

void SendBufferChunk::Init() {
	_isOpen = false;
	_usedSize = 0;
}

shared_ptr<SendBuffer> SendBufferChunk::Open(uint32_t allocSize) {
	//���������� SEND_BUFFER_CHUNKũ�⺸�� ū ����Ʈ�� send�� �� ����
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);

	//���� ���� �̹� �����ִ°��� ���� �߸��Ǿ���.
	//�ʱ�ȭ�� �߸���ų�, Close()�� �������� �ʾҴ�.
	ASSERT_CRASH(_isOpen == false);

	//Manager���� Open�Ҷ� �� ��츦 üũ�� �־�����, Ȥ�� �𸣴� ����üũ
	if (allocSize > FreeSize())
		return nullptr;

	_isOpen = true;
	shared_ptr<SendBuffer> SendBufferRef = { objectPool<SendBuffer>::alloc(), objectPool<SendBuffer>::dealloc };
	SendBufferRef->Init(shared_from_this(), Index(), allocSize);
	return SendBufferRef;
}

void SendBufferChunk::Close(uint32_t allocSize) {
	ASSERT_CRASH(_isOpen == true);
	_isOpen = false;
	_usedSize += allocSize;
}

void SendBuffer::Init(shared_ptr<SendBufferChunk> chunkRef, unsigned char* index, uint32_t allocSize) {
	_chunkRef = chunkRef;
	_index = index;
	_allocSize = allocSize;
	_writeSize = 0;
}

void SendBuffer::Close(uint32_t writeSize) {
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_chunkRef->Close(writeSize);
}

shared_ptr<SendBuffer> SendBufferManager::Open(uint32_t allocSize) {
	//���������� SEND_BUFFER_CHUNKũ�⺸�� ū ����Ʈ�� send�� �� ����
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);

	if (LSendBufferChunkRef == nullptr or LSendBufferChunkRef->FreeSize() < allocSize) {
		shared_ptr<SendBufferChunk> newChunk = { objectPool<SendBufferChunk>::alloc(), objectPool<SendBufferChunk>::dealloc };
		LSendBufferChunkRef = newChunk;
	}
	ASSERT_CRASH(LSendBufferChunkRef->IsOpen() == false);

	return LSendBufferChunkRef->Open(allocSize);
}

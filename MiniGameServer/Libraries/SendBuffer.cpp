#include "pch.h"
#include "SendBuffer.h"

void SendBufferChunk::Init() {
	_isOpen = false;
	_usedSize = 0;
}

shared_ptr<SendBuffer> SendBufferChunk::Open(uint32_t allocSize) {
	//구조적으로 SEND_BUFFER_CHUNK크기보다 큰 바이트는 send할 수 없음
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);

	//열기 전에 이미 열려있는것은 무언가 잘못되었다.
	//초기화가 잘못됬거나, Close()가 동작하지 않았다.
	ASSERT_CRASH(_isOpen == false);

	//Manager에서 Open할때 이 경우를 체크해 주었으나, 혹시 모르니 더블체크
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
	//구조적으로 SEND_BUFFER_CHUNK크기보다 큰 바이트는 send할 수 없음
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);

	if (LSendBufferChunkRef == nullptr or LSendBufferChunkRef->FreeSize() < allocSize) {
		shared_ptr<SendBufferChunk> newChunk = { objectPool<SendBufferChunk>::alloc(), objectPool<SendBufferChunk>::dealloc };
		LSendBufferChunkRef = newChunk;
	}
	ASSERT_CRASH(LSendBufferChunkRef->IsOpen() == false);

	return LSendBufferChunkRef->Open(allocSize);
}

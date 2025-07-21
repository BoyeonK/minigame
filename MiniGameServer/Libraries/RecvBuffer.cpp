#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(uint32_t bufferSize) : _bufferSize(bufferSize) {
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);
}

void RecvBuffer::Clean() {
	int32_t remainedBufSize = DataSize();
	if (remainedBufSize == 0) 
		_readPos = _writePos = 0;
	else {
		if (FreeSize() < _bufferSize) {
			::memcpy(&_buffer[0], &_buffer[_readPos], remainedBufSize);
			_readPos = 0;
			_writePos = remainedBufSize;
		}
	}
}

bool RecvBuffer::OnWrite(int32_t numOfBytes) {
	//Ŀ�ο��� �޾� �� ũ�⺸��, �����ִ� Recv������ ũ�Ⱑ �۴ٸ� false.
	if (numOfBytes > FreeSize())
		return false;

	//�ƴϸ� true�� ��ȯ.
	_writePos += numOfBytes;
	return true;
}

bool RecvBuffer::OnRead(int32_t numOfBytes) {
	//���� �� ��ü�� �Ѿ� �� ������ ũ�Ⱑ
	//Recv�ްڴٰ� ��õ� ũ�⺸�� �۴ٸ�
	//��� ������ ������ �Ѿ���� �ʾҴٴ� ������, false��ȯ
	if (numOfBytes > DataSize())
		return false;

	//�׷��� ���� ���, ��� ���۰� �Ѿ�Դٰ� �Ǵ��ϰ�
	//OnRead�� ȣ���� �ڵ忡�� numOfBytes��ŭ�� ���۸� �ҷ��� ó�� �� ������ ����ǹǷ�
	//�̸�ŭ�� ������ ó�� �� ������ ����� _readPos�� ������ ����ش�.
	_readPos += numOfBytes;
	return true;
}


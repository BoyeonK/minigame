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
	//커널에서 받아 올 크기보다, 남아있는 Recv버퍼의 크기가 작다면 false.
	if (numOfBytes > FreeSize())
		return false;

	//아니면 true를 반환.
	_writePos += numOfBytes;
	return true;
}

bool RecvBuffer::OnRead(int32_t numOfBytes) {
	//현재 이 객체에 넘어 온 버퍼의 크기가
	//Recv받겠다고 명시된 크기보다 작다면
	//모든 정보가 온전히 넘어오지 않았다는 것으로, false반환
	if (numOfBytes > DataSize())
		return false;

	//그렇지 않은 경우, 모든 버퍼가 넘어왔다고 판단하고
	//OnRead를 호출한 코드에서 numOfBytes만큼의 버퍼를 불러와 처리 할 것으로 예상되므로
	//이만큼의 정보는 처리 될 것으로 여기고 _readPos를 앞으로 당겨준다.
	_readPos += numOfBytes;
	return true;
}


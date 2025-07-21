#pragma once

class RecvBuffer {
	enum { BUFFER_COUNT = 5 };
public:
	RecvBuffer(uint32_t bufferSize);
	~RecvBuffer() { }

	void Clean();
	bool OnRead(int32_t numOfBytes);
	bool OnWrite(int32_t numOfBytes);

	//사실상 _buffer의 data영역에 포인터를 반환하고 있기 때문에 
	//public으로 열어놓은 거나 다름없기는 하다.
	unsigned char* WritePos() { return &_buffer[_writePos]; }
	unsigned char* ReadPos() { return &_buffer[_readPos]; }
	int32_t DataSize() { return _writePos - _readPos; }
	int32_t FreeSize() { return _capacity - _writePos; }

private:
	uint32_t _capacity = 0;
	uint32_t _bufferSize = 0;
	uint32_t _writePos = 0;
	uint32_t _readPos = 0;
	vector<unsigned char> _buffer;
};


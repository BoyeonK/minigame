#pragma once

class RecvBuffer {
	enum { BUFFER_COUNT = 5 };
public:
	RecvBuffer(uint32_t bufferSize);
	~RecvBuffer() { }

	void Clean();
	bool OnRead(int32_t numOfBytes);
	bool OnWrite(int32_t numOfBytes);

	//��ǻ� _buffer�� data������ �����͸� ��ȯ�ϰ� �ֱ� ������ 
	//public���� ������� �ų� �ٸ������ �ϴ�.
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


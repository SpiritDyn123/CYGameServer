#ifndef BUFFER_H
#define BUFFER_H

#include <Windows.h>
#include <iostream>
#include <vector>

#include <assert.h>
#include <string.h>

template<int SIZE>
class FixedBuffer
{
public:
	FixedBuffer() :
		_cur(_data)
	{

	}

	const char *data()
	{
		return _data;
	}

	int length()
	{
		return (int)(_cur - _data);
	}

	int avail()
	{
		return SIZE - (int)length();
	}

	char *current() const { return _cur;  }

	void append(const char *data, int nLen)
	{
		if (data == NULL || avail() < nLen)
			return;

		memcpy(_cur, data, nLen);
		_cur += nLen;
	}

	void addLen(int nLen)
	{
		_cur += nLen;
	}

	 int Size()
	{
		return SIZE;
	}

	 void retrieveLen(int nLen)
	{
		if (nLen > length())
			nLen = length();

		_cur -= nLen;
	}

private:
	char _data[SIZE];
	char *_cur;
};

class Buffer
{
public:
	Buffer() :
		_buffer(KBufferPrependSize + KBufferInitSize),
		_readIndex(KBufferPrependSize),
		_writeIndex(KBufferPrependSize)
	{

	}

	Buffer(int len) :
		_buffer(KBufferPrependSize + (len > 0 ? len : KBufferInitSize)),
		_readIndex(KBufferPrependSize),
		_writeIndex(KBufferPrependSize)
	{

	}

	Buffer(Buffer &buf)
	{
		_buffer.swap(buf._buffer);
		std::swap(_readIndex, buf._readIndex);
		std::swap(_writeIndex, buf._writeIndex);
	}

	void Resize(int nLen)
	{
		if (nLen <= 0)
			return;

		_buffer.resize(nLen + KBufferPrependSize);
		retrieveAll();
	}

	int readIndex() const
	{
		return _readIndex;
	}

	int writerIndex() const
	{
		return _writeIndex;
	}

	int writableBytes() const
	{
		return _buffer.size() - _writeIndex;
	}

	int readableBytes() const
	{
		return _writeIndex - _readIndex;
	}

	int prependabelBytes() const
	{
		return _readIndex;
	}

	void retrieveAll()
	{
		_readIndex = KBufferPrependSize;
		_writeIndex = KBufferPrependSize;
	}

	void retrieve(int nLen)
	{
		if (nLen < readableBytes())
		{
			_readIndex += nLen;
		}
		else
		{
			retrieveAll();
		}
	}
	
	char * begin()
	{
		return &*_buffer.begin();
	}

	char * beginWrite()
	{
		return begin() + _writeIndex;
	}

	char * beginRead()
	{
		return begin() + _readIndex;
	}

public:
	void write(char * data, int nLen)
	{
		if (data == NULL || nLen <= 0)
			return;

		ensureWritable(nLen);

		std::copy(data, data + nLen, beginWrite());
		_writeIndex += nLen;
	}

	char * read(char *data, int nLen)
	{
		if (nLen > readableBytes() || data == NULL)
			return NULL;

		memcpy(data, beginRead(), nLen);

		return data;
	}

	UINT8 readUInt8()
	{
		UINT8 data = 0;
		read((char *)&data, sizeof(UINT8));
		return data;
	}

	UINT16 readUInt16()
	{
		UINT16 data = 0;
		read((char *)&data, sizeof(UINT16));
		return data;
	}

	UINT32 readUInt32()
	{
		UINT32 data = 0;
		read((char *)&data, sizeof(UINT32));
		return data;
	}

	int readInt()
	{
		int data = 0;
		read((char *)&data, sizeof(int));
		return data;
	}

private:
	void makeSpace(int nLen)
	{
		if (prependabelBytes() + writableBytes() < nLen + KBufferPrependSize)
		{
			_buffer.resize(_writeIndex + nLen);
		}
		else
		{
			int readableLen = readableBytes();
			std::copy(beginRead(), beginRead() + readableLen, begin() + KBufferPrependSize);
			_readIndex = KBufferPrependSize;
			_writeIndex = _readIndex + readableLen;
		}
	}

	void ensureWritable(int nLen)
	{
		if (writableBytes() < nLen)
		{
			makeSpace(nLen);
		}
	}

private:
	static const int KBufferPrependSize = 8;
	static const int KBufferInitSize = 1024;
private:
	std::vector<char> _buffer;
	int          _readIndex;
	int          _writeIndex;
};

#endif
#ifndef CLOG_STREAM_H
#define CLOG_STREAM_H

#include <iostream>
#include "Buffer.h"

#define KStreamBuffSize 1024

class CLogStream
{
public:
	typedef FixedBuffer<KStreamBuffSize> Buffer;
	typedef CLogStream self;

	Buffer &buffer() { return _buffer; }

	self& operator<<(char v)
	{
		_buffer.append(&v, 1);
		return *this;
	}

	self& operator <<(const char *data)
	{
		if (data)
		{
			_buffer.append(data, strlen(data));
		}

		return *this;
	}

	self& operator <<(const std::string &data)
	{
		_buffer.append(data.c_str(), data.size());
		return *this;
	}

	self& operator<<(float v)
	{
		*this << static_cast<double>(v);
		return *this;
	}

	self& operator<<(short);
	self& operator<<(unsigned short);
	self& operator<<(int);
	self& operator<<(unsigned int);
	self& operator<<(long);
	self& operator<<(unsigned long);
	self& operator<<(long long);
	self& operator<<(unsigned long long);
	self& operator<<(const void*);
	self& operator<<(double);

	template<typename T>
	void formatInterger(T v);


public:
	static const int KMaxNumricSize = 32;

private:
	Buffer _buffer;
};

#endif
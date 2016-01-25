#include "CLogStream.h"


const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

const char digitsHex[] = "0123456789ABCDEF";

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
int convert(char buf[], T value)
{
	T i = value;
	char* p = buf;

	do
	{
		int lsd = static_cast<int>(i % 10);
		i /= 10;
		*p++ = zero[lsd];
	} while (i != 0);

	if (value < 0)
	{
		*p++ = '-';
	}
	*p = '\0';
	std::reverse(buf, p);

	return p - buf;
}

int convertHex(char buf[], uintptr_t value)
{
	uintptr_t i = value;
	char* p = buf;

	do
	{
		int lsd = static_cast<int>(i % 16);
		i /= 16;
		*p++ = digitsHex[lsd];
	} while (i != 0);

	*p = '\0';
	std::reverse(buf, p);

	return p - buf;
}

template<typename T>
void CLogStream::formatInterger(T v)
{
	if (_buffer.avail() >= KMaxNumricSize)
	{
		int len = convert(_buffer.current(), v);
		_buffer.addLen(len);
	}
}

CLogStream& CLogStream::operator<<(short data)
{
	*this << static_cast<int>(data);
	return *this;
}

CLogStream& CLogStream::operator<<(unsigned short data)
{
	*this << static_cast<unsigned int>(data);
	return *this;
}

CLogStream& CLogStream::operator<<(int data)
{
	formatInterger(data);
	return *this;
}

CLogStream& CLogStream::operator<<(unsigned int data)
{
	formatInterger(data);
	return *this;
}

CLogStream& CLogStream::operator<<(long data)
{
	formatInterger(data);
	return *this;
}

CLogStream& CLogStream::operator<<(unsigned long data)
{
	formatInterger(data);
	return *this;
}

CLogStream& CLogStream::operator<<(long long data)
{
	formatInterger(data);
	return *this;
}

CLogStream& CLogStream::operator<<(unsigned long long data)
{
	formatInterger(data);
	return *this;
}

CLogStream& CLogStream::operator<<(const void *p)
{
	uintptr_t v = reinterpret_cast<uintptr_t>(p);
	if (_buffer.avail() >= KMaxNumricSize)
	{
		char* buf = _buffer.current();
		buf[0] = '0';
		buf[1] = 'x';
		size_t len = convertHex(buf + 2, v);
		_buffer.addLen(len + 2);
	}
	return *this;
}

CLogStream& CLogStream::operator<<(double data)
{
	if (_buffer.avail() >= KMaxNumricSize)
	{
		int nLen = sprintf_s(_buffer.current(), KMaxNumricSize, "%12g", data);
		_buffer.addLen(nLen);
	}

	return *this;
}

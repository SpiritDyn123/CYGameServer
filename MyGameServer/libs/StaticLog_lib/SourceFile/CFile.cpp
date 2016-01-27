#include "CFile.h"
#include <time.h>
#include <assert.h>

CFile::CFile(std::string fileName)
:_fileName(fileName),
_writtenBytes(0)
{
	_fp = fopen(_fileName.c_str(), "a+");
	assert(_fp != NULL);
}

CFile::CFile() :
_fp(NULL),
_writtenBytes(0)
{

}

CFile::~CFile()
{
	if (_fp)
	{
		fclose(_fp);
	}
}

void CFile::append(const char *data, int nLen)
{
	if (data == NULL || nLen <= 0)
		return;

	assert(_fp != NULL);

	int len = fwrite(data, 1, nLen, _fp);
	if (len > 0)
	{
		_writtenBytes += len;
	}
}

void CFile::flush()
{
	fflush(_fp);
}

void CFile::resetFile(std::string fileName)
{
	if (_fp)
		fclose(_fp);

	_fileName = fileName;
	_fp = fopen(_fileName.c_str(), "a+");
	assert(_fp != NULL);
	_writtenBytes = 0;
}

CLogFile::CLogFile(std::string baseFileName, int rolFileSize, int flushInterval/* = 3*/) :
_baseFileName(baseFileName),
_rolFileSize(rolFileSize),
_flushInterval(flushInterval),
_lastFlushTime(0),
_lastRolTime(0),
_lastDay(0)
{
	RollFile();
}

CLogFile::~CLogFile()
{

}

std::string CLogFile::GetRollFileName(std::string baseName, time_t &now)
{
	now = time(NULL);

	tm t;
	localtime_s(&t, &now);

	std::string fileName(baseName);
	char timeStr[32];
	sprintf_s(timeStr, 32, "%4d-%02d-%02d_%02d_%02d_%02d"
		, t.tm_year + 1900
		, t.tm_mon + 1
		, t.tm_mday
		, t.tm_hour
		, t.tm_min
		, t.tm_sec);
	
	fileName += timeStr;
	fileName += ".log";

	return fileName;
}

void CLogFile::append(const char *data, int nLen)
{
	if (data == NULL || nLen <= 0)
		return;

	_curFile.append(data, nLen);

	if (_curFile.writtenBytes() > (DWORD)_rolFileSize)
	{
		RollFile();
	}
	else
	{
		time_t now = time(NULL);
		time_t curDay = now / KOneDayTotalSec * KOneDayTotalSec;
		if (_lastDay != curDay)
		{
			RollFile();
		}
		else
		{
			if (now - _lastFlushTime > _flushInterval)
			{
				flush();
				_lastFlushTime = now;
			}
		}
	}
}

void CLogFile::flush()
{
	_curFile.flush();
	_lastFlushTime = time(NULL);
}

void CLogFile::RollFile()
{
	time_t now;
	std::string fileName = GetRollFileName(_baseFileName, now);
	_curFile.resetFile(fileName);
	_lastDay = now / KOneDayTotalSec * KOneDayTotalSec;
	_lastRolTime = now;
	_lastFlushTime = now;
}

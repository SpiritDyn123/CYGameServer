#ifndef CFILE_H
#define CFILE_H

#include <Windows.h>
#include <time.h>
#include <iostream>

class CFile
{
public:
	CFile(std::string fileName);
	CFile();
	~CFile();

	void append(const char *data, int nLen);

	void flush();

	DWORD writtenBytes() { return _writtenBytes; }

	void resetFile(std::string fileName);

private:
	std::string _fileName;
	FILE* _fp;
	DWORD _writtenBytes;
};

class CLogFile
{
public:
	CLogFile(std::string baseFileName, int rolFileSize, int flushInterval = 3);
	~CLogFile();

	void append(const char *data, int nLen);

	void flush();

	void RollFile();
public:
	
	std::string GetRollFileName(std::string baseName, time_t &now);

private:
	std::string _baseFileName;
	int _rolFileSize;
	int _flushInterval;

	CFile _curFile;

	time_t _lastFlushTime;
	time_t _lastRolTime;
	time_t _lastDay;
    
	const int KOneDayTotalSec = 24 * 60 * 60;

};

#endif
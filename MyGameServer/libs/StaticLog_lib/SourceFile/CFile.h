#ifndef CFILE_H
#define CFILE_H

#include <Windows.h>
#include <time.h>
#include <iostream>

using namespace std;

class CFile
{
public:
	CFile(string fileName);
	CFile();
	~CFile();

	void append(const char *data, int nLen);

	void flush();

	DWORD writtenBytes() { return _writtenBytes; }

	void resetFile(string fileName);

private:
	string _fileName;
	FILE* _fp;
	DWORD _writtenBytes;
};

class CLogFile
{
public:
	CLogFile(string baseFileName, int rolFileSize, int flushInterval = 3);
	~CLogFile();

	void append(const char *data, int nLen);

	void flush();

	void RollFile();
public:
	
	static string GetRollFileName(string baseName, time_t &now);

private:
	string _baseFileName;
	int _rolFileSize;
	int _flushInterval;

	CFile _curFile;

	time_t _lastFlushTime;
	time_t _lastRolTime;
	time_t _lastDay;
    
	static const int KOneDayTotalSec = 24 * 60 * 60;

};

#endif
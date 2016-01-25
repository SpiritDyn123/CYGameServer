#ifndef CLOG_TOFILE_THREAD_H
#define CLOG_TOFILE_THREAD_H

#include <Windows.h>
#include <iostream>
#include <vector>
#include "ILog.h"
#include "Buffer.h"

#define LOG_PER_BUFF_SIZE 4000 * 1024
//根据每天0点和_rolFileSize大小来重置文件
class CLogToFileThread
{
public:
	CLogToFileThread(std::string fileName, int rolFileSize, int flushInterval = 3);
	~CLogToFileThread();

	void append(const char *data, int nLen);

	void Start();

	void Stop();

public:
	static DWORD WINAPI threadFunc(LPVOID lparm);

	typedef FixedBuffer<LOG_PER_BUFF_SIZE> LogBuffer;
	typedef std::vector<LogBuffer *> VEC_LOGBUFFER;
	typedef std::vector<LogBuffer *>::iterator VEC_LOGBUFFER_ITER;
private:
	std::string _fileName;
	int _rolFileSize;
	int _flushInterval;//秒
	bool _running;

	DWORD _threadId;
	HANDLE _threadHd;
	HANDLE _readyEvent;
	HANDLE _mutex;

	VEC_LOGBUFFER _vecBuff;
	LogBuffer *_currentBuffer;

	HANDLE _exitEvent;
};

#endif
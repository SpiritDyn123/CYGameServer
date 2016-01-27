#ifndef CLOG_TOFILE_THREAD_H
#define CLOG_TOFILE_THREAD_H

#include <Windows.h>
#include <iostream>
#include <vector>
#include "ILog.h"
#include "Buffer.h"
#include "CThread.h"

#define LOG_PER_BUFF_SIZE 4 * 1024
//����ÿ��0���_rolFileSize��С�������ļ�
class CLogToFileThread
{
public:
	CLogToFileThread(std::string fileName, int rolFileSize, int flushInterval = 3);
	~CLogToFileThread();

	void append(const char *data, int nLen);

	void Start();

	void Stop();

public:
	void threadFunc(void * data);//�̻߳ص�����

	typedef FixedBuffer<LOG_PER_BUFF_SIZE> LogBuffer;
	typedef std::vector<LogBuffer *> VEC_LOGBUFFER;
	typedef std::vector<LogBuffer *>::iterator VEC_LOGBUFFER_ITER;
private:
	std::string _fileName;
	int _rolFileSize;
	int _flushInterval;//��
	bool _running;

	CThread _thread;

	HANDLE _readyEvent;
	HANDLE _mutex;

	VEC_LOGBUFFER _vecBuff;
	LogBuffer *_currentBuffer;
};

#endif
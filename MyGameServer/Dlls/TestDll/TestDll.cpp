// TestDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ITest.h"
#include "CThread.h"
#include "CTestDllImpted.h"


class CTestSys : public ITestSys
{
public:
	CTestSys() :
		_running(false)
	{

	}

	virtual void CreateThread()
	{
		_running = true;
		_thread = new CThread(std::bind(&CTestSys::threadFunc, this, std::placeholders::_1));
		if (_thread)
		{
			_thread->start();
		}
	}

	virtual void StopThread()
	{
		if (_thread)
		{
			_running = false;
			_thread->stop();
			delete _thread;
			_thread = NULL;
		}
	}

private:
	void threadFunc(void *data)
	{
		std::cout << "CTestSys::threadFunc" << std::endl;
		while (_running)
		{

		}
	}

private:
	CThread *_thread;
	bool _running;
};

extern "C" __declspec(dllexport) void CreateTestSys(ITestSys **pTest)
{
	*pTest = new CTestSys;
}

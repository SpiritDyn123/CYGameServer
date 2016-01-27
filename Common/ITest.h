#ifndef ITEST_H
#define ITEST_H

#include <Windows.h>
#include <iostream>

class ITestSys
{
public:
	virtual void CreateThread() = NULL;

	virtual void StopThread() = NULL;

	virtual void TestLibFunc() = NULL;
};

class CTestHelper
{
public:
	typedef void(*CreateTestFun)(ITestSys **pTest);
	CTestHelper() :
		_hDll(NULL)
	{

	}

	~CTestHelper()
	{
		Release();
	}

	void Create()
	{
		Release();
		try
		{
			_hDll = LoadLibrary("TestDll.dll");
			if (_hDll == NULL)
			{
				throw "Can't load TestDll.dll";
			}

			CreateTestFun proc = (CreateTestFun)GetProcAddress(_hDll, "CreateTestSys");
			if (proc == NULL)
				throw "Can't GetProcAddress('CreateTestSys')";

			proc(&_pTest);
		}
		catch (LPCSTR errMgs)
		{
			throw errMgs;
		}
	}

	void Release()
	{
		if (_hDll != NULL)
		{
			::FreeLibrary(_hDll);
			_hDll = NULL;
		}
	}

	ITestSys * operator->()
	{
		return _pTest;
	}

	ITestSys *GetTestsys()
	{
		return _pTest;
	}

private:
	HINSTANCE _hDll;
	ITestSys *_pTest;
};
#endif
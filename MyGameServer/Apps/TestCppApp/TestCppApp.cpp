// TestCppApp.cpp : 定义控制台应用程序的入口点。
//

#define DLL_EXPORT __declspec(dllimport)

#include "stdafx.h"
#include "ITest.h"
#include "../../Dlls/TestDll/CTestDllImpted.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CTestHelper testHp;
	testHp.Create();

	return 0;
}


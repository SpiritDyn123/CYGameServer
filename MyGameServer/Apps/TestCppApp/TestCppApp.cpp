// TestCppApp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#define DLL_EXPORT __declspec(dllimport)

#include "stdafx.h"
#include "ITest.h"
#include "CTestDllImpted.h"
#include "../../Dlls/TestDll/CTestDllImpted.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CTestHelper testHp;
	testHp.Create();
	CTestImpted obj;
	obj.func();

	return 0;
}


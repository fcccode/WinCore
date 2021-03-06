/*
Copyright (c) 2013 Stijn "tcpie" Hinterding (contact: contact at tcpie dot eu)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "stdafx.h"

#include "..\WinCore\Function.h"
#include "..\WinCore\Thread.h"
#include "..\WinCore\Process.h"
#include "..\WinCore\MemoryRegion.h"
#include "..\WinCore\NotifyHook.h"

#include "HookTest.h"
#include "NotifyHookTest.h"

#include <vector>
#include <iostream>
#include <string>

#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\WinCore.lib")
#else
#pragma comment(lib, "..\\Release\\WinCore.lib")
#endif

using namespace tcpie::wincore;
using namespace std;

float __cdecl multiply(float a, int b)
{
	float ret = (float)b * a;
	float* ret_addr = &ret;
	return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
	hook_test();

	Function mult = Function(multiply,
									CDECL_CALLCONV,
									2,
									DWORD_SIZE);

	vector<void*> args = vector<void*>();
	float first = 3.0f;
	args.push_back((void*)5);
	args.push_back(*(void**)&first);
	

	DWORD ret = 0;
	DWORD* ret_addr = &ret;
	mult.Call(&args,
			  &ret);

	cout << hex << ret << dec << endl;

	notifyhook_test();

	MessageBoxA(NULL, "", "wait", MB_OK);

	return 0;


}


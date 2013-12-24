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
#include "..\WinCore\Hook.h"
#include "..\WinCore\NotifyHook.h"

#include <vector>
#include <iostream>
#include <string>

#pragma comment(lib, "..\\Release\\WinCore.lib")

using namespace tcpie::wincore;
using namespace std;

float __cdecl multiply(float a, int b)
{
	float ret = (float)b * a;
	float* ret_addr = &ret;
	return ret;
}

__declspec(noinline) int printLOL(int val)
{
	cout << "LOL " << val << endl;

	return val - 2;
}

void __stdcall notify_callback(void* fn_addr)
{
	cout << hex << "Notified of call of fn @0x" << fn_addr << dec << endl;
}

int __cdecl add(int a, float b)
{
	return a + (int)b;
}

DetourRet __stdcall add_change_arg(DetourArgs* args)
{
	cout << "add detour" << endl;

	args->Arguments->at(0) = (void*)4;

	return DETOUR_ARGCHANGED;
}

DetourRet __stdcall add_change_ret(DetourArgs* args)
{
	args->CustomReturnValue = 500;

	return DETOUR_RETCHANGED;
}

bool hook_test()
{
	cout << "Hook test:" << endl;
	vector<BYTE>* signature = new std::vector<BYTE>((BYTE*)add, (BYTE*)add_change_arg);
	vector<char>* mask = new std::vector<char>(signature->size(), 'x');

	Function* target = Function::FindFunction(signature,
		mask,
		CDECL_CALLCONV,			// Calling convention
		2,						// Number of arguments
		DWORD_SIZE);			// Return type

	cout << "\tFindFunction() succeeded? " << (bool)(target != NULL) << endl;

	Hook* cdecl_hook = Hook::CreateHook(target, new wstring(L"add"));

	cout << "\tCreateHook() succeeded? " << (bool)(cdecl_hook != NULL) << endl;

	Detour* cdecl_change_arg_detour = cdecl_hook->RegisterDetour(add_change_arg, DETOUR_PRE);

	cout << "\tRegisterDetour(fn) succeeded? " << (bool)(cdecl_change_arg_detour != NULL) << endl;

	Detour* cdecl_change_ret_detour = cdecl_hook->RegisterDetour(add_change_ret, DETOUR_POST);

	cdecl_hook->Enable();
	cdecl_change_ret_detour->Disable();

	int ret = add(2, 5);

	cout << "\tChange arg succeeded? " << (bool)(ret == 9) << endl;
	cdecl_change_ret_detour->Enable();
	cdecl_change_arg_detour->Disable();

	ret = add(2, 5);

	cout << "\tChange ret succeeded? " << (bool)(ret == 500) << endl;

	return true;
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

	NotifyHook* n_hook = NotifyHook::CreateHook(&printLOL, new wstring(L"printLOL"));
	n_hook->Enable();

	ret = printLOL(10);

	cout << ret << endl;

	MessageBoxA(NULL, "", "wait", MB_OK);

	return 0;


}


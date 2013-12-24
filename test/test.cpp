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

int __cdecl add(int a, int b)
{
	return a + b;
}

DetourRet __stdcall add_detour(DetourArgs* args)
{
	args->Arguments->at(0) = (void*)4;
		
	return DETOUR_ARGCHANGED;
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	vector<BYTE>* signature = new std::vector<BYTE>((BYTE*)add, (BYTE*)add_detour);
	vector<char>* mask = new std::vector<char>(signature->size(), 'x');

	Function* target = Function::FindFunction(signature,
									mask,
									CDECL_CALLCONV,			// Calling convention
									2,						// Number of arguments
									DWORD_SIZE);			// Return type
	
	Hook* hook = Hook::CreateHook(target,					// Function
								  new wstring(L"add"));		// Hook name
	
	Detour* our_detour = hook->RegisterDetour(add_detour,	// Detour callback
											  DETOUR_PRE);	// Detour type (pre / post)
	
	hook->Enable();
	
	cout << add(2, 5) << endl;								// Output: "9"

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


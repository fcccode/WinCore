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

#include <vector>
#include <iostream>
#include <string>

#pragma comment(lib, "..\\Debug\\WinCore.lib")

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

int _tmain(int argc, _TCHAR* argv[])
{
	Function* target = new Function(add,					// Address
									CDECL_CALLCONV,			// Calling convention
									2,						// Number of arguments
									DWORD_SIZE);			// Return type
	
	Hook* hook = Hook::CreateHook(target,					// Function
								  new wstring(L"add"));		// Hook name
	
	Detour* our_detour = hook->RegisterDetour(add_detour,	// Detour callback
											  DETOUR_PRE);	// Detour type (pre / post)
	
	hook->Enable();
	
	cout << add(2, 5) << endl;								// Output: "9"

	MessageBoxA(NULL, "", "wait", MB_OK);

	return 0;


}

